/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/events.h>
#include <beckon/status_transport.h>
#include <raw_hid/events.h>
#include <zmk/split/central.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static void pack_three_bits(uint8_t *data, size_t bit, uint8_t value) {
    size_t byte = bit / 8;
    uint8_t offset = bit % 8;
    data[byte] |= value << offset;
    if (offset > 5) {
        data[byte + 1] |= value >> (8 - offset);
    }
}

static int raw_hid_status_listener(const zmk_event_t *eh) {
    struct raw_hid_received_event *event = as_raw_hid_received_event(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    struct beckon_status_snapshot snapshot;
    int err = beckon_status_transport_decode(event->data, event->length, &snapshot);
    if (err) {
        LOG_WRN("Rejected Beckon status report: %d", err);
        return ZMK_EV_EVENT_BUBBLE;
    }

    // AIDEV-NOTE: Raw HID owns event->data. Raise only copied declarative
    // state so a future RGB consumer cannot retain a USB callback buffer.
    err = raise_beckon_status_snapshot_received(
        (struct beckon_status_snapshot_received){.snapshot = snapshot});
    if (err) {
        return err;
    }

#if IS_ENABLED(CONFIG_BECKON_STATUS_SPLIT_SYNC)
    struct zmk_split_transport_beckon_status split_status = {.sequence = snapshot.sequence};
    for (size_t i = 0; i < BECKON_STATUS_SLOT_COUNT; i++) {
        pack_three_bits(split_status.slots, i * 3, snapshot.slots[i]);
    }
    for (size_t i = 0; i < BECKON_STATUS_TREATMENT_COUNT; i++) {
        const struct beckon_status_treatment *treatment = &snapshot.treatments[i];
        split_status.treatments[i * 2] = (treatment->red & 0xf0) | (treatment->green >> 4);
        split_status.treatments[i * 2 + 1] = (treatment->blue & 0xf0) |
                                            (treatment->brightness >> 4);
        pack_three_bits(split_status.motions, i * 3, treatment->motion);
    }
    err = zmk_split_central_update_beckon_status(&split_status);
    if (err && err != -ENODEV) {
        LOG_WRN("Failed to sync Beckon status to split peripheral: %d", err);
    }
#endif

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(beckon_raw_hid_status, raw_hid_status_listener);
ZMK_SUBSCRIPTION(beckon_raw_hid_status, raw_hid_received_event);
