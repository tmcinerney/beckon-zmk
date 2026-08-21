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

bool beckon_status_led_color(enum beckon_agent_status status, uint32_t *rgb) {
    if (!rgb) {
        return false;
    }

    switch (status) {
    case BECKON_AGENT_STATUS_UNBOUND:
        return false;
    case BECKON_AGENT_STATUS_IDLE:
        *rgb = BECKON_IDLE_RGB;
        return true;
    case BECKON_AGENT_STATUS_WORKING:
        *rgb = BECKON_WORKING_RGB;
        return true;
    case BECKON_AGENT_STATUS_BLOCKED:
        *rgb = BECKON_BLOCKED_RGB;
        return true;
    case BECKON_AGENT_STATUS_DONE:
        *rgb = BECKON_DONE_RGB;
        return true;
    case BECKON_AGENT_STATUS_UNKNOWN:
        *rgb = BECKON_UNKNOWN_RGB;
        return true;
    }

    return false;
}

bool beckon_status_led_should_render(bool beckon_layer_active, enum beckon_agent_status status,
                                     uint32_t *rgb) {
    return beckon_layer_active && beckon_status_led_color(status, rgb);
}
