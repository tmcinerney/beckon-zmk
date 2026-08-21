/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/events.h>
#include <beckon/status_split_sync.h>

#include <string.h>

int zmk_split_peripheral_handle_beckon_status(
    const struct zmk_split_transport_beckon_status *status) {
    if (!status) {
        return -EINVAL;
    }

    struct beckon_status_snapshot snapshot = {
        .sequence = status->sequence,
    };
    memcpy(snapshot.slots, status->slots, sizeof(snapshot.slots));

    return raise_beckon_status_snapshot_received(
        (struct beckon_status_snapshot_received){.snapshot = snapshot});
}
