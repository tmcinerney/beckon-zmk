/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/status_led.h>

enum {
    BECKON_IDLE_RGB = 0x3BA0FF,
    BECKON_WORKING_RGB = 0x00C48C,
    BECKON_BLOCKED_RGB = 0xFF2B2B,
    BECKON_DONE_RGB = 0xFFFFFF,
    BECKON_UNKNOWN_RGB = 0xFFB000,
};

static uint8_t cap_channel(uint8_t channel) {
    return (uint16_t)channel * CONFIG_BECKON_STATUS_LED_BRIGHTNESS_PERCENT / 100;
}

static uint32_t cap_rgb(uint32_t rgb) {
    return ((uint32_t)cap_channel((rgb >> 16) & 0xff) << 16) |
           ((uint32_t)cap_channel((rgb >> 8) & 0xff) << 8) | cap_channel(rgb & 0xff);
}

bool beckon_status_led_color(enum beckon_agent_status status, uint32_t *rgb) {
    if (!rgb) {
        return false;
    }

    switch (status) {
    case BECKON_AGENT_STATUS_UNBOUND:
        return false;
    case BECKON_AGENT_STATUS_IDLE:
        *rgb = cap_rgb(BECKON_IDLE_RGB);
        return true;
    case BECKON_AGENT_STATUS_WORKING:
        *rgb = cap_rgb(BECKON_WORKING_RGB);
        return true;
    case BECKON_AGENT_STATUS_BLOCKED:
        *rgb = cap_rgb(BECKON_BLOCKED_RGB);
        return true;
    case BECKON_AGENT_STATUS_DONE:
        *rgb = cap_rgb(BECKON_DONE_RGB);
        return true;
    case BECKON_AGENT_STATUS_UNKNOWN:
        *rgb = cap_rgb(BECKON_UNKNOWN_RGB);
        return true;
    }

    return false;
}

bool beckon_status_led_should_render(bool beckon_layer_active, enum beckon_agent_status status,
                                     uint32_t *rgb) {
    return beckon_layer_active && beckon_status_led_color(status, rgb);
}
