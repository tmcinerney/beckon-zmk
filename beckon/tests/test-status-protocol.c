/* SPDX-License-Identifier: MIT */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <beckon/status_transport.h>

static void pack_three_bits(uint8_t frame[BECKON_STATUS_REPORT_SIZE], size_t bit, uint8_t value) {
    size_t byte = bit / 8;
    uint8_t offset = bit % 8;
    frame[byte] |= value << offset;
    if (offset > 5) {
        frame[byte + 1] |= value >> (8 - offset);
    }
}

static void fill_valid_frame(uint8_t frame[BECKON_STATUS_REPORT_SIZE]) {
    memset(frame, 0, BECKON_STATUS_REPORT_SIZE);
    frame[0] = BECKON_STATUS_PROTOCOL_VERSION;
    frame[1] = BECKON_STATUS_MESSAGE_SNAPSHOT;
    frame[2] = 42;
    pack_three_bits(frame, 4 * 8, BECKON_AGENT_STATUS_IDLE);
    pack_three_bits(frame, 4 * 8 + 3, BECKON_AGENT_STATUS_WORKING);
    pack_three_bits(frame, 4 * 8 + 6, BECKON_AGENT_STATUS_BLOCKED);
    pack_three_bits(frame, 4 * 8 + 9, BECKON_AGENT_STATUS_DONE);
    pack_three_bits(frame, 4 * 8 + 12, BECKON_AGENT_STATUS_UNKNOWN);
    frame[8] = 0x11;
    frame[9] = 0x22;
    frame[10] = 0x33;
    frame[23] = 0x80;
    pack_three_bits(frame, 28 * 8, BECKON_STATUS_MOTION_BREATHE);
}

int main(void) {
    uint8_t frame[BECKON_STATUS_REPORT_SIZE];
    struct beckon_status_snapshot snapshot;

    fill_valid_frame(frame);
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == 0);
    assert(snapshot.sequence == 42);
    assert(snapshot.slots[0] == BECKON_AGENT_STATUS_IDLE);
    assert(snapshot.slots[4] == BECKON_AGENT_STATUS_UNKNOWN);
    assert(snapshot.treatments[0].red == 0x11);
    assert(snapshot.treatments[0].green == 0x22);
    assert(snapshot.treatments[0].blue == 0x33);
    assert(snapshot.treatments[0].brightness == 0x80);
    assert(snapshot.treatments[0].motion == BECKON_STATUS_MOTION_BREATHE);

    assert(beckon_status_transport_decode(frame, sizeof(frame) - 1, &snapshot) == -EMSGSIZE);
    frame[0] = BECKON_STATUS_PROTOCOL_VERSION + 1;
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == -EPROTONOSUPPORT);

    fill_valid_frame(frame);
    frame[1] = 0;
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == -ENOTSUP);

    fill_valid_frame(frame);
    pack_three_bits(frame, 4 * 8, BECKON_AGENT_STATUS_UNKNOWN + 1);
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == -EINVAL);

    fill_valid_frame(frame);
    frame[30] = 1;
    assert(beckon_status_transport_decode(frame, sizeof(frame), &snapshot) == -EINVAL);
    return 0;
}
