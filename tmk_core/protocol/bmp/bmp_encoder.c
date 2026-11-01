// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

// QMK
#include "encoder.h"
#include "keyboard.h"
#include "print.h"

// BMP
#include "apidef.h"
#include "bmp.h"

#ifndef ENCODER_RESOLUTION
#    define ENCODER_RESOLUTION 4
#endif

#ifndef ENCODER_SCANTIME_MS
#    define ENCODER_SCANTIME_MS 2
#endif

#define ENCODER_CLOCKWISE true
#define ENCODER_COUNTER_CLOCKWISE false
#define NUM_ENCODERS_MAX 8

// Split slave sends encoder steps to master as user data (id = base + encoder index)
#define ENCODER_USER_DATA_ID_BASE 0xE0
#define REMOTE_EVENT_QUEUE_SIZE 16

static int    encoder_count                    = 0;
static int8_t encoder_value[NUM_ENCODERS_MAX]  = {0};
static int8_t encoder_unsent[NUM_ENCODERS_MAX] = {0};

typedef struct {
    uint8_t index;
    int8_t  steps; // positive: clockwise
} remote_encoder_event_t;

static remote_encoder_event_t remote_event_queue[REMOTE_EVENT_QUEUE_SIZE];
static volatile uint8_t       remote_event_head = 0;
static volatile uint8_t       remote_event_tail = 0;

static void encoder_exec_mapping(uint8_t index, bool clockwise);

__attribute__((weak)) bool encoder_update_user(uint8_t index, bool clockwise) {
    return true;
}

__attribute__((weak)) bool encoder_update_kb(uint8_t index, bool clockwise) {
    bool res = encoder_update_user(index, clockwise);
    if (res) {
        encoder_exec_mapping(index, clockwise);
    }
    return res;
}

static void encoder_exec_mapping(uint8_t index, bool clockwise) {
    // The delays below cater for Windows and its wonderful requirements.
    action_exec(clockwise ? MAKE_ENCODER_CW_EVENT(index, true) : MAKE_ENCODER_CCW_EVENT(index, true));
#if ENCODER_MAP_KEY_DELAY > 0
    wait_ms(ENCODER_MAP_KEY_DELAY);
#endif // ENCODER_MAP_KEY_DELAY > 0

    action_exec(clockwise ? MAKE_ENCODER_CW_EVENT(index, false) : MAKE_ENCODER_CCW_EVENT(index, false));
#if ENCODER_MAP_KEY_DELAY > 0
    wait_ms(ENCODER_MAP_KEY_DELAY);
#endif // ENCODER_MAP_KEY_DELAY > 0
}

// Called by BMP API when user data arrives from split slave. Only enqueue here.
static void encoder_user_data_cb(uint8_t id, uint8_t data) {
    if (id < ENCODER_USER_DATA_ID_BASE || id >= ENCODER_USER_DATA_ID_BASE + NUM_ENCODERS_MAX) {
        return;
    }

    uint8_t next = (remote_event_head + 1) % REMOTE_EVENT_QUEUE_SIZE;
    if (next == remote_event_tail) {
        return; // queue full, drop
    }

    remote_event_queue[remote_event_head].index = id - ENCODER_USER_DATA_ID_BASE;
    remote_event_queue[remote_event_head].steps = (int8_t)data;
    remote_event_head                           = next;
}

static bool encoder_process_remote(void) {
    bool changed = false;

    while (remote_event_tail != remote_event_head) {
        remote_encoder_event_t ev = remote_event_queue[remote_event_tail];
        remote_event_tail         = (remote_event_tail + 1) % REMOTE_EVENT_QUEUE_SIZE;

        // remote encoders are numbered after local ones
        uint8_t index = encoder_count + ev.index;
        printf("Remote encoder %d: %d\n", index, ev.steps);

        for (int8_t i = 0; i < ev.steps; i++) {
            encoder_update_kb(index, ENCODER_CLOCKWISE);
        }
        for (int8_t i = 0; i > ev.steps; i--) {
            encoder_update_kb(index, ENCODER_COUNTER_CLOCKWISE);
        }
        changed = true;
    }

    return changed;
}

// Steps that could not be sent (e.g. link busy) are kept and retried on next scan
static void encoder_send_to_master(uint8_t index, int8_t steps) {
    int16_t unsent = encoder_unsent[index] + steps;
    unsent         = unsent > INT8_MAX ? INT8_MAX : unsent < INT8_MIN ? INT8_MIN : unsent;

    if (BMPAPI->ble.send_user_data_s2m == NULL) {
        return;
    }

    if (unsent != 0 && BMPAPI->ble.send_user_data_s2m(ENCODER_USER_DATA_ID_BASE + index, (uint8_t)unsent) == BMP_OK) {
        unsent = 0;
    }
    encoder_unsent[index] = unsent;
}

void encoder_init(void) {
    if (bmp_config->mode == SPLIT_MASTER && BMPAPI->ble.set_user_data_cb != NULL) {
        BMPAPI->ble.set_user_data_cb(encoder_user_data_cb);
    }

    for (int i = 0; i < NUM_ENCODERS_MAX; i++) {
        if (bmp_config->encoder.pin_a[i] == 0 || bmp_config->encoder.pin_b[i] == 0) {
            break;
        }
        encoder_count++;
    }

    if (encoder_count > 0) {
        BMPAPI->encoder.init(ENCODER_SCANTIME_MS, bmp_config->encoder.pin_a, bmp_config->encoder.pin_b, encoder_count);
    }
}

bool encoder_task(void) {
    bool changed  = false;
    bool is_slave = bmp_config->mode == SPLIT_SLAVE;

    if (bmp_config->mode == SPLIT_MASTER) {
        changed = encoder_process_remote();
    }

    for (int index = 0; index < encoder_count; index++) {
        int32_t encoder_diff = BMPAPI->encoder.get_count(index);
        encoder_value[index] += encoder_diff;

        int8_t resolution = bmp_config->encoder.resolution[index];
        resolution        = resolution > 0 ? resolution : ENCODER_RESOLUTION;

        if (is_slave) {
            int8_t steps = 0;
            if (encoder_value[index] >= resolution) {
                steps = -1;
            } else if (encoder_value[index] <= -resolution) {
                steps = 1;
            }
            encoder_send_to_master(index, steps);
        } else if (encoder_value[index] >= resolution) {
            encoder_update_kb(index, ENCODER_COUNTER_CLOCKWISE);
            changed = true;
        } else if (encoder_value[index] <= -resolution) {
            encoder_update_kb(index, ENCODER_CLOCKWISE);
            changed = true;
        }

        encoder_value[index] %= resolution;
    }

    return changed;
}
