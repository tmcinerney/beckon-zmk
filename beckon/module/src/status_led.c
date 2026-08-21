/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/events.h>
#include <beckon/status_led.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>
#else
#include <zmk/events/split_peripheral_layer_changed.h>
#endif

#include <zmk/rgb_underglow.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static enum beckon_agent_status current_status = BECKON_AGENT_STATUS_UNBOUND;
static bool beckon_layer_active;

static void refresh_status_led(void) {
    uint32_t rgb;
    int err;

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    const uint8_t pixel = BECKON_GLOVE80_LEFT_F1_PIXEL;
#else
    const uint8_t pixel = BECKON_GLOVE80_RIGHT_F6_PIXEL;
#endif

    if (beckon_status_led_should_render(beckon_layer_active, current_status, &rgb)) {
        err = zmk_rgb_underglow_override_pixel(pixel, rgb);
    } else {
        err = zmk_rgb_underglow_clear_pixel_override(pixel);
    }
    if (err) {
        LOG_WRN("Failed to update Beckon status LED: %d", err);
    }
}

static int beckon_status_led_listener(const zmk_event_t *eh) {
    const struct beckon_status_snapshot_received *event = as_beckon_status_snapshot_received(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    current_status = event->snapshot.slots[0];
#else
    current_status = event->snapshot.slots[5];
#endif
    refresh_status_led();

    return ZMK_EV_EVENT_BUBBLE;
}

static int beckon_layer_state_listener(const zmk_event_t *eh) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    if (!as_zmk_layer_state_changed(eh)) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    beckon_layer_active = zmk_keymap_layer_active(CONFIG_BECKON_STATUS_LED_LAYER);
#else
    const struct zmk_split_peripheral_layer_changed *event =
        as_zmk_split_peripheral_layer_changed(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    beckon_layer_active = (event->layers & BIT(CONFIG_BECKON_STATUS_LED_LAYER)) != 0;
#endif
    refresh_status_led();

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(beckon_status_led, beckon_status_led_listener);
ZMK_SUBSCRIPTION(beckon_status_led, beckon_status_snapshot_received);
ZMK_LISTENER(beckon_layer_state, beckon_layer_state_listener);
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
ZMK_SUBSCRIPTION(beckon_layer_state, zmk_layer_state_changed);
#else
ZMK_SUBSCRIPTION(beckon_layer_state, zmk_split_peripheral_layer_changed);
#endif
