/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#define BECKON_STATUS_PROTOCOL_VERSION 1
#define BECKON_STATUS_REPORT_SIZE 32
#define BECKON_STATUS_SLOT_COUNT 10

enum beckon_status_message_type {
    BECKON_STATUS_MESSAGE_SNAPSHOT = 1,
};

enum beckon_agent_status {
    BECKON_AGENT_STATUS_UNBOUND = 0,
    BECKON_AGENT_STATUS_IDLE = 1,
    BECKON_AGENT_STATUS_WORKING = 2,
    BECKON_AGENT_STATUS_BLOCKED = 3,
    BECKON_AGENT_STATUS_DONE = 4,
    BECKON_AGENT_STATUS_UNKNOWN = 5,
};

struct beckon_status_snapshot {
    uint8_t sequence;
    enum beckon_agent_status slots[BECKON_STATUS_SLOT_COUNT];
};

/*
 * Decode a fixed-size, status-only snapshot. The caller owns `out`; no raw
 * HID callback memory escapes this boundary.
 */
int beckon_status_transport_decode(const uint8_t *data, size_t length,
                                   struct beckon_status_snapshot *out);
