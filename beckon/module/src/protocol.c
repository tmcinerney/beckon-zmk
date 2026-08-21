/*
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <stdbool.h>

#include <beckon/status_transport.h>

enum {
    PROTOCOL_VERSION_OFFSET = 0,
    MESSAGE_TYPE_OFFSET = 1,
    SEQUENCE_OFFSET = 2,
    RESERVED_OFFSET = 3,
    SLOT_STATES_OFFSET = 4,
    SLOT_STATES_BITS = 30,
    TREATMENT_COLORS_OFFSET = 8,
    TREATMENT_BRIGHTNESS_OFFSET = 23,
    TREATMENT_MOTIONS_OFFSET = 28,
    RESERVED_TRAILER_OFFSET = 30,
};

static bool is_valid_status(uint8_t status) { return status <= BECKON_AGENT_STATUS_UNKNOWN; }
static bool is_valid_motion(uint8_t motion) { return motion <= BECKON_STATUS_MOTION_FLICKER; }

static uint8_t unpack_three_bits(const uint8_t *data, size_t bit) {
    size_t byte = bit / 8;
    uint8_t offset = bit % 8;
    uint8_t value = data[byte] >> offset;
    if (offset > 5) {
        value |= data[byte + 1] << (8 - offset);
    }
    return value & 0x07;
}

int beckon_status_transport_decode(const uint8_t *data, size_t length,
                                   struct beckon_status_snapshot *out) {
    if (!data || !out) {
        return -EINVAL;
    }
    if (length != BECKON_STATUS_REPORT_SIZE) {
        return -EMSGSIZE;
    }
    if (data[PROTOCOL_VERSION_OFFSET] != BECKON_STATUS_PROTOCOL_VERSION) {
        return -EPROTONOSUPPORT;
    }
    if (data[MESSAGE_TYPE_OFFSET] != BECKON_STATUS_MESSAGE_SNAPSHOT) {
        return -ENOTSUP;
    }
    if (data[RESERVED_OFFSET] != 0) {
        return -EINVAL;
    }

    for (size_t i = 0; i < BECKON_STATUS_SLOT_COUNT; i++) {
        if (!is_valid_status(unpack_three_bits(data, SLOT_STATES_OFFSET * 8 + i * 3))) {
            return -EINVAL;
        }
    }
    if ((data[SLOT_STATES_OFFSET + SLOT_STATES_BITS / 8] & 0xc0) != 0) {
        return -EINVAL;
    }
    for (size_t i = 0; i < BECKON_STATUS_TREATMENT_COUNT; i++) {
        if (!is_valid_motion(unpack_three_bits(data, TREATMENT_MOTIONS_OFFSET * 8 + i * 3))) {
            return -EINVAL;
        }
    }
    if ((data[TREATMENT_MOTIONS_OFFSET + 1] & 0xc0) != 0) {
        return -EINVAL;
    }
    for (size_t i = RESERVED_TRAILER_OFFSET; i < BECKON_STATUS_REPORT_SIZE; i++) {
        if (data[i] != 0) {
            return -EINVAL;
        }
    }

    out->sequence = data[SEQUENCE_OFFSET];
    for (size_t i = 0; i < BECKON_STATUS_SLOT_COUNT; i++) {
        out->slots[i] =
            (enum beckon_agent_status)unpack_three_bits(data, SLOT_STATES_OFFSET * 8 + i * 3);
    }
    for (size_t i = 0; i < BECKON_STATUS_TREATMENT_COUNT; i++) {
        size_t color_offset = TREATMENT_COLORS_OFFSET + i * 3;
        out->treatments[i] = (struct beckon_status_treatment){
            .red = data[color_offset],
            .green = data[color_offset + 1],
            .blue = data[color_offset + 2],
            .brightness = data[TREATMENT_BRIGHTNESS_OFFSET + i],
            .motion = (enum beckon_status_motion)unpack_three_bits(
                data, TREATMENT_MOTIONS_OFFSET * 8 + i * 3),
        };
    }
    return 0;
}
