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

#include <string.h>

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static enum beckon_agent_status current_status[BECKON_STATUS_LED_COUNT_PER_HALF];
static struct beckon_status_treatment current_treatments[BECKON_STATUS_TREATMENT_COUNT];
static bool beckon_layer_active;
static bool animation_running;

static void refresh_status_led(void);

static void animation_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    refresh_status_led();
}

K_WORK_DEFINE(animation_work, animation_work_handler);

static void animation_timer_handler(struct k_timer *timer) {
    ARG_UNUSED(timer);
    k_work_submit(&animation_work);
}

K_TIMER_DEFINE(animation_timer, animation_timer_handler, NULL);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static const uint8_t status_led_pixels[BECKON_STATUS_LED_COUNT_PER_HALF] = {
    BECKON_GLOVE80_LEFT_F1_PIXEL, BECKON_GLOVE80_LEFT_F2_PIXEL, BECKON_GLOVE80_LEFT_F3_PIXEL,
    BECKON_GLOVE80_LEFT_F4_PIXEL, BECKON_GLOVE80_LEFT_F5_PIXEL,
};
#define BECKON_STATUS_SLOT_OFFSET 0
#else
static const uint8_t status_led_pixels[BECKON_STATUS_LED_COUNT_PER_HALF] = {
    BECKON_GLOVE80_RIGHT_F6_PIXEL, BECKON_GLOVE80_RIGHT_F7_PIXEL,  BECKON_GLOVE80_RIGHT_F8_PIXEL,
    BECKON_GLOVE80_RIGHT_F9_PIXEL, BECKON_GLOVE80_RIGHT_F10_PIXEL,
};
#define BECKON_STATUS_SLOT_OFFSET 5
#endif

static void refresh_status_led(void) {
    bool animate = false;
    for (size_t index = 0; index < ARRAY_SIZE(status_led_pixels); index++) {
        uint32_t rgb;
        int err;
        if (beckon_status_led_should_render(beckon_layer_active, current_status[index],
                                            current_treatments, &rgb)) {
            enum beckon_agent_status status = current_status[index];
            enum beckon_status_motion motion = current_treatments[status - 1].motion;
            rgb = beckon_status_led_apply_motion(rgb, motion, k_uptime_get_32());
            animate = animate || motion != BECKON_STATUS_MOTION_STEADY;
            err = zmk_rgb_underglow_override_pixel(status_led_pixels[index], rgb);
        } else {
            err = zmk_rgb_underglow_clear_pixel_override(status_led_pixels[index]);
        }
        if (err) {
            LOG_WRN("Failed to update Beckon status LED %u: %d", index, err);
        }
    }
    if (animate && !animation_running) {
        k_timer_start(&animation_timer, K_MSEC(50), K_MSEC(50));
        animation_running = true;
    } else if (!animate && animation_running) {
        k_timer_stop(&animation_timer);
        animation_running = false;
    }
}

static int beckon_status_led_listener(const zmk_event_t *eh) {
    const struct beckon_status_snapshot_received *event = as_beckon_status_snapshot_received(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    memcpy(current_status, &event->snapshot.slots[BECKON_STATUS_SLOT_OFFSET],
           sizeof(current_status));
    memcpy(current_treatments, event->snapshot.treatments, sizeof(current_treatments));
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
