// Copyright 2024-2026 George Norton / LXF-YZP
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "procyon_touchpad.h"
#include "split_util.h"
#include "timer.h"
#include "action.h"
#include <stdlib.h>
#include <string.h>

#ifndef TOUCHPAD_DEFAULT_CPI
#    define TOUCHPAD_DEFAULT_CPI 800
#endif

#ifndef TOUCHPAD_MAX_DELTA
#    define TOUCHPAD_MAX_DELTA 35
#endif

#ifndef TOUCHPAD_SCROLL_DIVISOR
#    define TOUCHPAD_SCROLL_DIVISOR 10
#endif

static report_mouse_t mouse_report = {0};
static uint16_t mouse_cpi = TOUCHPAD_DEFAULT_CPI;

uint16_t touchpad_get_cpi(void) {
    return mouse_cpi;
}

void touchpad_set_cpi(uint16_t cpi) {
    mouse_cpi = cpi;
}

report_mouse_t touchpad_get_mouse_report(report_mouse_t _mouse_report) {
    report_mouse_t report = mouse_report;
    memset(&mouse_report, 0, sizeof(report_mouse_t));
    return report;
}

// Simple and robust state machine: Move, Scroll, or None (no taps = no stuck buttons!)
typedef enum { STATE_NONE, STATE_MOVE, STATE_SCROLL } TouchState;

static TouchState state = STATE_NONE;

bool touchpad_update_gesture_state(void) {
    return state != STATE_NONE;
}

void touchpad_update_mouse_report(touchpad_report_t *report) {
    static uint16_t last_x = 0;
    static uint16_t last_y = 0;
    const uint16_t  x      = report->fingers[0].x * (TOUCHPAD_RESOLUTION_X / TOUCHPAD_WIDTH_MM) / mouse_cpi;
    const uint16_t  y      = report->fingers[0].y * (TOUCHPAD_RESOLUTION_Y / TOUCHPAD_HEIGHT_MM) / mouse_cpi;
    int             contacts = 0;

    memset(&mouse_report, 0, sizeof(report_mouse_t));

    for (int i = 0; i < TOUCHPAD_FINGER_COUNT; i++) {
        if (report->fingers[i].tip) {
            contacts++;
        }
    }

    if (contacts == 0) {
        state = STATE_NONE;
    } else if (contacts == 1) {
        if (state != STATE_MOVE) {
            state = STATE_MOVE;
            last_x = x;
            last_y = y;
        } else {
            int dx = (int)x - (int)last_x;
            int dy = (int)y - (int)last_y;
#ifdef TOUCHPAD_MAX_DELTA
            if (dx > TOUCHPAD_MAX_DELTA) dx = TOUCHPAD_MAX_DELTA;
            if (dx < -TOUCHPAD_MAX_DELTA) dx = -TOUCHPAD_MAX_DELTA;
            if (dy > TOUCHPAD_MAX_DELTA) dy = TOUCHPAD_MAX_DELTA;
            if (dy < -TOUCHPAD_MAX_DELTA) dy = -TOUCHPAD_MAX_DELTA;
#endif
            mouse_report.x = (int8_t)dx;
            mouse_report.y = (int8_t)dy;
            last_x = x;
            last_y = y;
        }
    } else {
        // 2 or more fingers: Scroll
        if (state != STATE_SCROLL) {
            state = STATE_SCROLL;
            last_x = x;
            last_y = y;
        } else {
            static int carry_h = 0;
            static int carry_v = 0;
            const int  h       = (int)x - (int)last_x + carry_h;
            const int  v       = (int)y - (int)last_y + carry_v;

            carry_h = h % TOUCHPAD_SCROLL_DIVISOR;
            carry_v = v % TOUCHPAD_SCROLL_DIVISOR;

            mouse_report.h = h / TOUCHPAD_SCROLL_DIVISOR;
            mouse_report.v = v / TOUCHPAD_SCROLL_DIVISOR;
            last_x = x;
            last_y = y;
        }
    }
}

// Pointing device custom driver interface
void pointing_device_driver_init(void) {
    if (is_keyboard_left()) {
        return;
    }
#ifdef DIGITIZER_MOTION_PIN
#ifdef DIGITIZER_MOTION_PIN_ACTIVE_LOW
    gpio_set_pin_input_high(DIGITIZER_MOTION_PIN);
#else
    gpio_set_pin_input(DIGITIZER_MOTION_PIN);
#endif
#endif
    maxtouch_init();
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t current_report) {
    if (is_keyboard_left()) {
        return current_report;
    }

    bool motion = true;
#ifdef DIGITIZER_MOTION_PIN
#ifdef DIGITIZER_MOTION_PIN_ACTIVE_LOW
    motion = !gpio_read_pin(DIGITIZER_MOTION_PIN);
#else
    motion = gpio_read_pin(DIGITIZER_MOTION_PIN);
#endif
#endif

    bool gesture_active = touchpad_update_gesture_state();
    if (!motion && !gesture_active) {
        report_mouse_t r = current_report;
        r.x = 0;
        r.y = 0;
        r.h = 0;
        r.v = 0;
        return r;
    }

    static touchpad_state_t touchpad_state = {0};
    touchpad_state_t driver_state = maxtouch_get_report(touchpad_state);
    touchpad_report_t report = {0};
    int skip_count = 0;

    for (int i = 0; i < TOUCHPAD_FINGER_COUNT; i++) {
        const bool finger_contact = (driver_state.contacts[i].type == TOUCHPAD_TYPE_FINGER && driver_state.contacts[i].tip) ||
                                    (touchpad_state.contacts[i].type == TOUCHPAD_TYPE_FINGER && touchpad_state.contacts[i].tip);
        const uint8_t finger_index = finger_contact ? report.contact_count : TOUCHPAD_FINGER_COUNT - skip_count - 1;

        if (finger_contact) {
            report.fingers[finger_index].tip = driver_state.contacts[i].tip;
            report.contact_count++;
        } else {
            skip_count++;
            report.fingers[finger_index].tip = false;
        }
        report.fingers[finger_index].contact_id = i;
        report.fingers[finger_index].x = driver_state.contacts[i].x;
        report.fingers[finger_index].y = driver_state.contacts[i].y;
        report.fingers[finger_index].confidence = driver_state.contacts[i].confidence;
    }
    touchpad_state = driver_state;

    touchpad_update_mouse_report(&report);

    report_mouse_t mouse = touchpad_get_mouse_report(current_report);
    // Keep button states exclusively from physical keyboard keys (never stuck from touchpad!)
    mouse.buttons = current_report.buttons;
    return mouse;
}

uint16_t pointing_device_driver_get_cpi(void) {
    return touchpad_get_cpi();
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
    touchpad_set_cpi(cpi);
}
