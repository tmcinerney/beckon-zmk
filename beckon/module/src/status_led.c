/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/events.h>
#include <beckon/status_led.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/rgb_underglow.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static enum beckon_agent_status current_status = BECKON_AGENT_STATUS_UNBOUND;

static void refresh_status_led(void) {
    const bool beckon_layer_active = zmk_keymap_layer_active(CONFIG_BECKON_STATUS_LED_LAYER);
    uint32_t rgb;
    int err;
    if (beckon_status_led_should_render(beckon_layer_active, current_status, &rgb)) {
        err = zmk_rgb_underglow_override_pixel(BECKON_GLOVE80_LEFT_F1_PIXEL, rgb);
    } else {
        err = zmk_rgb_underglow_clear_pixel_override(BECKON_GLOVE80_LEFT_F1_PIXEL);
    }
    if (err) {
        LOG_WRN("Failed to update Beckon F1 status LED: %d", err);
    }
}

static int beckon_status_led_listener(const zmk_event_t *eh) {
    const struct beckon_status_snapshot_received *event = as_beckon_status_snapshot_received(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    current_status = event->snapshot.slots[0];
    refresh_status_led();

    return ZMK_EV_EVENT_BUBBLE;
}

static int beckon_layer_state_listener(const zmk_event_t *eh) {
    if (!as_zmk_layer_state_changed(eh)) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    refresh_status_led();

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(beckon_status_led, beckon_status_led_listener);
ZMK_SUBSCRIPTION(beckon_status_led, beckon_status_snapshot_received);
ZMK_LISTENER(beckon_layer_state, beckon_layer_state_listener);
ZMK_SUBSCRIPTION(beckon_layer_state, zmk_layer_state_changed);
