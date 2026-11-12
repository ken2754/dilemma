// Copyright 2024-2026 George Norton / LXF-YZP
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "report.h"
#include "procyon.h"

#ifndef TOUCHPAD_FINGER_COUNT
#    define TOUCHPAD_FINGER_COUNT 5
#endif

#ifdef MXT_SWITCH_XY
#    define TOUCHPAD_WIDTH_MM MXT_SENSOR_HEIGHT_MM
#    define TOUCHPAD_HEIGHT_MM MXT_SENSOR_WIDTH_MM
#else
#    define TOUCHPAD_WIDTH_MM MXT_SENSOR_WIDTH_MM
#    define TOUCHPAD_HEIGHT_MM MXT_SENSOR_HEIGHT_MM
#endif

#ifndef MXT_SAMPLES_PER_MM
#    define MXT_SAMPLES_PER_MM 100
#endif

#ifndef TOUCHPAD_RESOLUTION_X
#    define TOUCHPAD_RESOLUTION_X (TOUCHPAD_WIDTH_MM * MXT_SAMPLES_PER_MM)
#endif
#ifndef TOUCHPAD_RESOLUTION_Y
#    define TOUCHPAD_RESOLUTION_Y (TOUCHPAD_HEIGHT_MM * MXT_SAMPLES_PER_MM)
#endif

#ifndef TOUCHPAD_CONTACT_COUNT
#    define TOUCHPAD_CONTACT_COUNT MAX(1, TOUCHPAD_FINGER_COUNT)
#endif

typedef enum { TOUCHPAD_TYPE_UNKNOWN, TOUCHPAD_TYPE_FINGER, TOUCHPAD_TYPE_STYLUS } touchpad_type_t;

typedef struct {
    touchpad_type_t  type;
    uint8_t          tip : 1;
    uint8_t          in_range : 1;
    uint8_t          confidence : 1;
    uint16_t         x;
    uint16_t         y;
} touchpad_contact_t;

typedef struct {
    touchpad_contact_t contacts[TOUCHPAD_CONTACT_COUNT];
    union {
        uint8_t buttons;
        struct {
            uint8_t button1 : 1;
            uint8_t button2 : 1;
            uint8_t button3 : 1;
        };
    };
} touchpad_state_t;

typedef struct {
    uint8_t  contact_id;
    uint8_t  tip : 1;
    uint8_t  in_range : 1;
    uint8_t  confidence : 1;
    uint16_t x;
    uint16_t y;
} touchpad_finger_t;

typedef struct {
    touchpad_finger_t fingers[TOUCHPAD_FINGER_COUNT];
    uint8_t           contact_count;
    union {
        uint8_t buttons;
        struct {
            uint8_t button1 : 1;
            uint8_t button2 : 1;
            uint8_t button3 : 1;
        };
    };
} touchpad_report_t;

void             maxtouch_init(void);
touchpad_state_t maxtouch_get_report(touchpad_state_t touchpad_report);

void             touchpad_update_mouse_report(touchpad_report_t *report);
report_mouse_t   touchpad_get_mouse_report(report_mouse_t _mouse_report);
bool             touchpad_update_gesture_state(void);
uint16_t         touchpad_get_cpi(void);
void             touchpad_set_cpi(uint16_t cpi);

void             pointing_device_driver_init(void);
report_mouse_t   pointing_device_driver_get_report(report_mouse_t mouse_report);
uint16_t         pointing_device_driver_get_cpi(void);
void             pointing_device_driver_set_cpi(uint16_t cpi);
