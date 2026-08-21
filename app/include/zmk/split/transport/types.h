/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/hid_indicators_types.h>
#include <zmk/sensors.h>
#include <zephyr/sys/util.h>

enum zmk_split_transport_connections_status {
    ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_DISCONNECTED = 0,
    ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_SOME_CONNECTED,
    ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_ALL_CONNECTED,
};

struct zmk_split_transport_status {
    bool available;
    bool enabled;
    enum zmk_split_transport_connections_status connections;
};

typedef struct zmk_split_transport_status (*zmk_split_transport_get_status_t)(void);
typedef int (*zmk_split_transport_set_enabled_t)(bool enabled);

enum zmk_split_transport_peripheral_event_type {
    ZMK_SPLIT_TRANSPORT_PERIPHERAL_EVENT_TYPE_KEY_POSITION_EVENT,
    ZMK_SPLIT_TRANSPORT_PERIPHERAL_EVENT_TYPE_SENSOR_EVENT,
    ZMK_SPLIT_TRANSPORT_PERIPHERAL_EVENT_TYPE_INPUT_EVENT,
    ZMK_SPLIT_TRANSPORT_PERIPHERAL_EVENT_TYPE_BATTERY_EVENT,
};

struct zmk_split_transport_peripheral_event {
    enum zmk_split_transport_peripheral_event_type type;

    union {
        struct {
            uint8_t position;
            uint8_t pressed;
        } key_position_event;

        struct {
            struct zmk_sensor_channel_data channel_data;

            uint8_t sensor_index;
        } sensor_event;

        struct {
            uint8_t reg;
            uint8_t sync;
            uint8_t type;
            uint16_t code;
            int32_t value;
        } input_event;

        struct {
            uint8_t level;
        } battery_event;
    } data;
} __packed;

enum zmk_split_transport_central_command_type {
    ZMK_SPLIT_TRANSPORT_CENTRAL_CMD_TYPE_POLL_EVENTS,
    ZMK_SPLIT_TRANSPORT_CENTRAL_CMD_TYPE_INVOKE_BEHAVIOR,
    ZMK_SPLIT_TRANSPORT_CENTRAL_CMD_TYPE_SET_PHYSICAL_LAYOUT,
    ZMK_SPLIT_TRANSPORT_CENTRAL_CMD_TYPE_SET_HID_INDICATORS,
    ZMK_SPLIT_TRANSPORT_CENTRAL_CMD_TYPE_SET_RGB_LAYERS,
    /*
     * Beckon sends its host snapshot to split peripherals over ZMK's already
     * encrypted split service. This is deliberately not a host-facing BLE
     * endpoint: only the USB-connected central can originate it.
     */
    ZMK_SPLIT_TRANSPORT_CENTRAL_CMD_TYPE_SET_BECKON_STATUS,
} __packed;

#define ZMK_SPLIT_TRANSPORT_BECKON_STATUS_SLOT_COUNT 10
#define ZMK_SPLIT_TRANSPORT_BECKON_STATUS_TREATMENT_COUNT 5

struct zmk_split_transport_beckon_status {
    uint8_t sequence;
    /* 3-bit status values: 10 values fit in four bytes. */
    uint8_t slots[4];
    /* Five RGB444 + brightness4 treatments: two bytes each. */
    uint8_t treatments[ZMK_SPLIT_TRANSPORT_BECKON_STATUS_TREATMENT_COUNT * 2];
    /* Five 3-bit motion values fit in two bytes. */
    uint8_t motions[2];
} __packed;

struct zmk_split_transport_central_command {
    enum zmk_split_transport_central_command_type type;

    union {
        struct {
            char behavior_dev[16];
            uint32_t param1, param2;
            uint32_t position;
            uint8_t event_source;
            uint8_t state;
        } invoke_behavior;

        struct {
            uint8_t layout_idx;
        } set_physical_layout;

        struct {
            zmk_hid_indicators_t indicators;
        } set_hid_indicators;

        struct {
            uint32_t layers;
        } set_rgb_layers;

        struct zmk_split_transport_beckon_status set_beckon_status;
    } data;
} __packed;
