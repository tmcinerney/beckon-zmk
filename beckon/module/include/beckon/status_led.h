/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <beckon/status_transport.h>

/*
 * The Glove80 left board's `pixel-lookup` maps matrix position 0 (F1) to
 * WS2812 strip index 34. Keep this hardware mapping here rather than in the
 * status protocol, which remains keyboard-neutral.
 */
#define BECKON_GLOVE80_LEFT_F1_PIXEL 34

/* The Glove80 right board's lookup maps matrix position 5 (F6) to strip 10. */
#define BECKON_GLOVE80_RIGHT_F6_PIXEL 10

/*
 * Return an RGB override for one declarative agent status. `false` means the
 * status has no display ownership and the normal layer RGB should show.
 */
bool beckon_status_led_color(enum beckon_agent_status status, uint32_t *rgb);

/* Beckon owns an indicator only while its keymap layer is active. */
bool beckon_status_led_should_render(bool beckon_layer_active, enum beckon_agent_status status,
                                     uint32_t *rgb);
