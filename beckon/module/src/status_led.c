/*
 * SPDX-License-Identifier: MIT
 */

#include <beckon/events.h>
#include <beckon/status_led.h>
#include <zmk/rgb_underglow.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static int beckon_status_led_listener(const zmk_event_t *eh) {
    const struct beckon_status_snapshot_received *event = as_beckon_status_snapshot_received(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint32_t rgb;
    int err;
    if (beckon_status_led_color(event->snapshot.slots[0], &rgb)) {
        err = zmk_rgb_underglow_override_pixel(BECKON_GLOVE80_LEFT_F1_PIXEL, rgb);
    } else {
        err = zmk_rgb_underglow_clear_pixel_override(BECKON_GLOVE80_LEFT_F1_PIXEL);
    }
    if (err) {
        LOG_WRN("Failed to update Beckon F1 status LED: %d", err);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(beckon_status_led, beckon_status_led_listener);
ZMK_SUBSCRIPTION(beckon_status_led, beckon_status_snapshot_received);
