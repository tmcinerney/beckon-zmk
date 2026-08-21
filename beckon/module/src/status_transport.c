/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/events.h>
#include <beckon/status_transport.h>
#include <raw_hid/events.h>
#include <zmk/split/central.h>

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

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
    memcpy(split_status.slots, snapshot.slots, sizeof(split_status.slots));
    err = zmk_split_central_update_beckon_status(&split_status);
    if (err && err != -ENODEV) {
        LOG_WRN("Failed to sync Beckon status to split peripheral: %d", err);
    }
#endif

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(beckon_raw_hid_status, raw_hid_status_listener);
ZMK_SUBSCRIPTION(beckon_raw_hid_status, raw_hid_received_event);
