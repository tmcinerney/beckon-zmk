/* SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdint.h>

#include <beckon/status_led.h>

int main(void) {
    uint32_t rgb = 0;

    assert(!beckon_status_led_color(BECKON_AGENT_STATUS_UNBOUND, &rgb));
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_IDLE, &rgb));
    assert(rgb == 0x2F80CC);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_WORKING, &rgb));
    assert(rgb == 0x009C70);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_BLOCKED, &rgb));
    assert(rgb == 0xCC2222);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_DONE, &rgb));
    assert(rgb == 0xCCCCCC);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_UNKNOWN, &rgb));
    assert(rgb == 0xCC8C00);
    assert(!beckon_status_led_color(BECKON_AGENT_STATUS_IDLE, NULL));
    assert(!beckon_status_led_color((enum beckon_agent_status)99, &rgb));

    assert(!beckon_status_led_should_render(false, BECKON_AGENT_STATUS_WORKING, &rgb));
    assert(beckon_status_led_should_render(true, BECKON_AGENT_STATUS_WORKING, &rgb));
    assert(rgb == 0x009C70);
    assert(!beckon_status_led_should_render(true, BECKON_AGENT_STATUS_UNBOUND, &rgb));
    return 0;
}
