/* SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdint.h>

#include <beckon/status_led.h>

int main(void) {
    uint32_t rgb = 0;
    const struct beckon_status_treatment treatments[BECKON_STATUS_TREATMENT_COUNT] = {
        {.red = 0xff, .green = 0, .blue = 0, .brightness = 0x80},
        {.red = 0, .green = 0xff, .blue = 0, .brightness = 0x80},
        {.red = 0, .green = 0, .blue = 0xff, .brightness = 0x80},
        {.red = 0xff, .green = 0xff, .blue = 0xff, .brightness = 0x80},
        {.red = 0x10, .green = 0x20, .blue = 0x30, .brightness = 0xff},
    };

    assert(!beckon_status_led_color(BECKON_AGENT_STATUS_UNBOUND, treatments, &rgb));
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_IDLE, treatments, &rgb));
    assert(rgb == 0x660000);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_WORKING, treatments, &rgb));
    assert(rgb == 0x006600);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_BLOCKED, treatments, &rgb));
    assert(rgb == 0x000066);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_DONE, treatments, &rgb));
    assert(rgb == 0x666666);
    assert(beckon_status_led_color(BECKON_AGENT_STATUS_UNKNOWN, treatments, &rgb));
    assert(rgb == 0x0c1926);
    assert(!beckon_status_led_color(BECKON_AGENT_STATUS_IDLE, treatments, NULL));
    assert(!beckon_status_led_color((enum beckon_agent_status)99, treatments, &rgb));

    assert(!beckon_status_led_should_render(false, BECKON_AGENT_STATUS_WORKING, treatments, &rgb));
    assert(beckon_status_led_should_render(true, BECKON_AGENT_STATUS_WORKING, treatments, &rgb));
    assert(rgb == 0x006600);
    assert(!beckon_status_led_should_render(true, BECKON_AGENT_STATUS_UNBOUND, treatments, &rgb));
    return 0;
}
