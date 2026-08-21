/* SPDX-License-Identifier: MIT */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <beckon/status_transport.h>

static void fill_valid_frame(uint8_t frame[BECKON_STATUS_REPORT_SIZE]) {
    memset(frame, 0, BECKON_STATUS_REPORT_SIZE);
    frame[0] = BECKON_STATUS_PROTOCOL_VERSION;
    frame[1] = BECKON_STATUS_MESSAGE_SNAPSHOT;
    frame[2] = 42;
    frame[4] = BECKON_AGENT_STATUS_IDLE;
    frame[5] = BECKON_AGENT_STATUS_WORKING;
    frame[6] = BECKON_AGENT_STATUS_BLOCKED;
    frame[7] = BECKON_AGENT_STATUS_DONE;
    frame[8] = BECKON_AGENT_STATUS_UNKNOWN;
}

int main(void) {
    uint8_t frame[BECKON_STATUS_REPORT_SIZE];
    struct beckon_status_snapshot snapshot;

    fill_valid_frame(frame);
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == 0);
    assert(snapshot.sequence == 42);
    assert(snapshot.slots[0] == BECKON_AGENT_STATUS_IDLE);
    assert(snapshot.slots[4] == BECKON_AGENT_STATUS_UNKNOWN);

    assert(beckon_status_transport_decode(frame, sizeof(frame) - 1, &snapshot) == -EMSGSIZE);
    frame[0] = BECKON_STATUS_PROTOCOL_VERSION + 1;
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == -EPROTONOSUPPORT);

    fill_valid_frame(frame);
    frame[1] = 0;
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == -ENOTSUP);

    fill_valid_frame(frame);
    frame[4] = BECKON_AGENT_STATUS_UNKNOWN + 1;
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == -EINVAL);

    fill_valid_frame(frame);
    frame[14] = 1;
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == -EINVAL);
    return 0;
}
