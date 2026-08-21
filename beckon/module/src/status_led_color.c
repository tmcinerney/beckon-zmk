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
                                     const struct beckon_status_treatment treatments[],
                                     uint32_t *rgb) {
    return beckon_layer_active && beckon_status_led_color(status, treatments, rgb);
}

static uint8_t scale_channel(uint8_t channel, uint8_t factor) {
    return (uint16_t)channel * factor / UINT8_MAX;
}

static uint8_t triangle(uint32_t phase, uint32_t period) {
    uint32_t point = phase % period;
    uint32_t half = period / 2;
    return point <= half ? point * UINT8_MAX / half : (period - point) * UINT8_MAX / half;
}

static uint8_t motion_factor(enum beckon_status_motion motion, uint32_t phase_ms) {
    switch (motion) {
    case BECKON_STATUS_MOTION_STEADY:
        return UINT8_MAX;
    case BECKON_STATUS_MOTION_BREATHE:
        return 48 + ((uint16_t)triangle(phase_ms, 1600) * 207 / UINT8_MAX);
    case BECKON_STATUS_MOTION_PULSE:
        return phase_ms % 1000 < 160 ? UINT8_MAX : 64;
    case BECKON_STATUS_MOTION_FLICKER:
        return ((phase_ms / 90) * 1103515245u + 12345u) & 0x80 ? UINT8_MAX : 96;
    }
    return UINT8_MAX;
}

uint32_t beckon_status_led_apply_motion(uint32_t rgb, enum beckon_status_motion motion,
                                        uint32_t phase_ms) {
    uint8_t factor = motion_factor(motion, phase_ms);
    return ((uint32_t)scale_channel((rgb >> 16) & 0xff, factor) << 16) |
           ((uint32_t)scale_channel((rgb >> 8) & 0xff, factor) << 8) |
           scale_channel(rgb & 0xff, factor);
}
