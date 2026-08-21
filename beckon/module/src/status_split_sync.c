/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/events.h>
#include <beckon/status_split_sync.h>

static uint8_t unpack_three_bits(const uint8_t *data, size_t bit) {
    size_t byte = bit / 8;
    uint8_t offset = bit % 8;
    uint8_t value = data[byte] >> offset;
    if (offset > 5) {
        value |= data[byte + 1] << (8 - offset);
    }
    return value & 0x07;
}

static uint8_t expand_nibble(uint8_t value) { return value * 0x11; }

int zmk_split_peripheral_handle_beckon_status(
    const struct zmk_split_transport_beckon_status *status) {
    if (!status) {
        return -EINVAL;
    }

    struct beckon_status_snapshot snapshot = {
        .sequence = status->sequence,
    };
    for (size_t i = 0; i < BECKON_STATUS_SLOT_COUNT; i++) {
        snapshot.slots[i] = (enum beckon_agent_status)unpack_three_bits(status->slots, i * 3);
    }
    for (size_t i = 0; i < BECKON_STATUS_TREATMENT_COUNT; i++) {
        uint8_t first = status->treatments[i * 2];
        uint8_t second = status->treatments[i * 2 + 1];
        snapshot.treatments[i] = (struct beckon_status_treatment){
            .red = expand_nibble(first >> 4),
            .green = expand_nibble(first & 0x0f),
            .blue = expand_nibble(second >> 4),
            .brightness = expand_nibble(second & 0x0f),
            .motion = (enum beckon_status_motion)unpack_three_bits(status->motions, i * 3),
        };
    }

    return raise_beckon_status_snapshot_received(
        (struct beckon_status_snapshot_received){.snapshot = snapshot});
}
