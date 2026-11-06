/*
This is the c configuration file for the keymap

Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifdef RGBLIGHT_ENABLE
#    define RGBLIGHT_EFFECT_BREATHING
//#    define RGBLIGHT_EFFECT_RAINBOW_MOOD
//#    define RGBLIGHT_EFFECT_RAINBOW_SWIRL
//#    define RGBLIGHT_EFFECT_SNAKE
//#    define RGBLIGHT_EFFECT_KNIGHT
//#    define RGBLIGHT_EFFECT_CHRISTMAS
#    define RGBLIGHT_EFFECT_STATIC_GRADIENT
//#    define RGBLIGHT_EFFECT_RGB_TEST
//#    define RGBLIGHT_EFFECT_ALTERNATING
//#    define RGBLIGHT_EFFECT_TWINKLE
// Brightness cap 50% (keyboard default 150). Breathing peaks at this value.
#    undef RGBLIGHT_LIMIT_VAL
#    define RGBLIGHT_LIMIT_VAL 128
#endif

#define TAP_CODE_DELAY 5

// Turn the OLED off after 10 s without key, ball or encoder input (default 60 s).
#define OLED_TIMEOUT 10000

// Upload SROM to PMW3360 at boot. Without it the sensor caps CPI at 3500.
#define KEYBALL_PMW3360_UPLOAD_SROM_ID 0x04

// Extra CLI commands (see pmw_cli.c)
#ifndef __ASSEMBLER__
#    include "microshell/core/msconf.h"
#    include "microshell/util/mscmd.h"
#    define USER_DEFINED_MSCMD {"pmw", usrcmd_pmw, "PMW3360 status / pmw <n>: set CPI reg"},
MSCMD_USER_RESULT usrcmd_pmw(MSOPT *msopt, MSCMD_USER_OBJECT usrobj);
#endif

#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 1

// Pins are set in CONFIG.BIN (encoder.pin_a/pin_b), not here
#define NUM_ENCODERS 2
