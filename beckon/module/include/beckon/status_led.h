/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <beckon/status_transport.h>

/*
 * Glove80 F-row LEDs, in Beckon key order on each physical half. The board
 * DTS remains the wiring source of truth; tests verify these derived values.
 */
#define BECKON_GLOVE80_LEFT_F1_PIXEL 34
#define BECKON_GLOVE80_LEFT_F2_PIXEL 28
#define BECKON_GLOVE80_LEFT_F3_PIXEL 22
#define BECKON_GLOVE80_LEFT_F4_PIXEL 16
#define BECKON_GLOVE80_LEFT_F5_PIXEL 10

#define BECKON_GLOVE80_RIGHT_F6_PIXEL 10
#define BECKON_GLOVE80_RIGHT_F7_PIXEL 16
#define BECKON_GLOVE80_RIGHT_F8_PIXEL 22
#define BECKON_GLOVE80_RIGHT_F9_PIXEL 28
#define BECKON_GLOVE80_RIGHT_F10_PIXEL 34

#define BECKON_STATUS_LED_COUNT_PER_HALF 5

/*
 * Return an RGB override for one declarative agent status. `false` means the
 * status has no display ownership and the normal layer RGB should show.
 */
bool beckon_status_led_color(enum beckon_agent_status status, uint32_t *rgb);

/* Beckon owns an indicator only while its keymap layer is active. */
bool beckon_status_led_should_render(bool beckon_layer_active, enum beckon_agent_status status,
                                     uint32_t *rgb);
