/* SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdint.h>

#include <beckon/status_led.h>

int main(void) {
    uint32_t rgb = 0;

    assert(!beckon_status_led_color(BECKON_AGENT_STATUS_UNBOUND, &rgb));
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_IDLE, &rgb));
    assert(rgb == 0x3BA0FF);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_WORKING, &rgb));
    assert(rgb == 0x00C48C);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_BLOCKED, &rgb));
    assert(rgb == 0xFF2B2B);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_DONE, &rgb));
    assert(rgb == 0xFFFFFF);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_UNKNOWN, &rgb));
    assert(rgb == 0xFFB000);
    assert(!beckon_status_led_color(BECKON_AGENT_STATUS_IDLE, NULL));
    assert(!beckon_status_led_color((enum beckon_agent_status)99, &rgb));
    return 0;
}
