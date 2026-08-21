/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/status_led.h>

static uint8_t cap_channel(uint8_t channel, uint8_t brightness) {
    uint16_t scaled = (uint16_t)channel * brightness / UINT8_MAX;
    return scaled * CONFIG_BECKON_STATUS_LED_BRIGHTNESS_PERCENT / 100;
}

static uint32_t treatment_rgb(const struct beckon_status_treatment *treatment) {
    return ((uint32_t)cap_channel(treatment->red, treatment->brightness) << 16) |
           ((uint32_t)cap_channel(treatment->green, treatment->brightness) << 8) |
           cap_channel(treatment->blue, treatment->brightness);
}

bool beckon_status_led_color(enum beckon_agent_status status,
                             const struct beckon_status_treatment treatments[], uint32_t *rgb) {
    if (!rgb || !treatments || status == BECKON_AGENT_STATUS_UNBOUND ||
        status > BECKON_AGENT_STATUS_UNKNOWN) {
        return false;
    }
    *rgb = treatment_rgb(&treatments[status - 1]);
    return true;
}

bool beckon_status_led_should_render(bool beckon_layer_active, enum beckon_agent_status status,
                                     const struct beckon_status_treatment treatments[], uint32_t *rgb) {
    return beckon_layer_active && beckon_status_led_color(status, treatments, rgb);
}
