/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

struct zmk_led_hsb {
    uint16_t h;
    uint8_t s;
    uint8_t b;
};

int zmk_rgb_underglow_toggle(void);
int zmk_rgb_underglow_get_state(bool *state);
int zmk_rgb_underglow_on(void);
int zmk_rgb_underglow_off(void);
int zmk_rgb_underglow_transient_on(void);
int zmk_rgb_underglow_transient_off(void);
int zmk_rgb_underglow_cycle_effect(int direction);
int zmk_rgb_underglow_calc_effect(int direction);
int zmk_rgb_underglow_select_effect(int effect);
struct zmk_led_hsb zmk_rgb_underglow_calc_hue(int direction);
struct zmk_led_hsb zmk_rgb_underglow_calc_sat(int direction);
struct zmk_led_hsb zmk_rgb_underglow_calc_brt(int direction);
int zmk_rgb_underglow_change_hue(int direction);
int zmk_rgb_underglow_change_sat(int direction);
int zmk_rgb_underglow_change_brt(int direction);
int zmk_rgb_underglow_change_spd(int direction);
int zmk_rgb_underglow_set_hsb(struct zmk_led_hsb color);
int zmk_rgb_underglow_status(void);

/*
 * Temporarily override one physical LED in the underglow strip. `rgb` is
 * encoded as 0xRRGGBB and is composed after the active layer's RGB map.
 * Overrides honour the user's RGB on/off setting and yield to the temporary
 * Magic status display. They are intentionally display-only; callers cannot
 * affect key behavior, layers, or transport state through this API.
 */
int zmk_rgb_underglow_override_pixel(uint8_t pixel, uint32_t rgb);
int zmk_rgb_underglow_clear_pixel_override(uint8_t pixel);
