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
};

static bool is_valid_status(uint8_t status) { return status <= BECKON_AGENT_STATUS_UNKNOWN; }

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
        if (!is_valid_status(data[SLOT_STATES_OFFSET + i])) {
            return -EINVAL;
        }
    }
    for (size_t i = SLOT_STATES_OFFSET + BECKON_STATUS_SLOT_COUNT; i < BECKON_STATUS_REPORT_SIZE;
         i++) {
        if (data[i] != 0) {
            return -EINVAL;
        }
    }

    out->sequence = data[SEQUENCE_OFFSET];
    for (size_t i = 0; i < BECKON_STATUS_SLOT_COUNT; i++) {
        out->slots[i] = (enum beckon_agent_status)data[SLOT_STATES_OFFSET + i];
    }
    return 0;
}
