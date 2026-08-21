/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <beckon/status_transport.h>
#include <zmk/event_manager.h>

struct beckon_status_snapshot_received {
    struct beckon_status_snapshot snapshot;
};

ZMK_EVENT_DECLARE(beckon_status_snapshot_received);
