/*
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

#include QMK_KEYBOARD_H

#include "quantum.h"
#include "state_controller.h"
#include "bmp_host_driver.h"

// The slave (left) must not send keys to the PC over its own USB.
// BMP switches to USB output when USB HID is ready before BLE connects, and the
// slave would then type with its own (unused) keymap. Keep it in BLE mode, the
// same state as running on battery. Vial/CLI/MSC over USB are not affected.
void bmp_state_change_cb_user(bmp_api_event_t event) {
    if (event == USB_HID_READY && !is_keyboard_master()) {
        select_ble();
    }
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default (VIA)
  [0] = LAYOUT_universal(
    KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                            KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     ,
    KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                            KC_H     , KC_J     , KC_K     , KC_L     , KC_MINS  ,
    KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     ,                            KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  ,
    KC_LCTL  , KC_LGUI  , KC_LALT  ,LSFT_T(KC_LNG2),LT(1,KC_SPC),LT(3,KC_LNG1),KC_BSPC,LT(2,KC_ENT),LSFT_T(KC_LNG2),KC_RALT,KC_RGUI, KC_RSFT
  ),

  [1] = LAYOUT_universal(
    KC_F1    , KC_F2    , KC_F3    , KC_F4    , KC_RBRC  ,                            KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   ,
    KC_F5    , KC_EXLM  , S(KC_6)  ,S(KC_INT3), S(KC_8)  ,                           S(KC_INT1), KC_BTN1  , KC_PGUP  , KC_BTN2  , KC_SCLN  ,
    S(KC_EQL),S(KC_LBRC),S(KC_7)   , S(KC_2)  ,S(KC_RBRC),                            KC_LBRC  , KC_DLR   , KC_PGDN  , KC_BTN3  , KC_F11   ,
    KC_INT1  , KC_EQL   , S(KC_3)  , _______  , _______  , _______  ,      TO(2)    , TO(0)    , _______  , KC_RALT  , KC_RGUI  , KC_F12
  ),

  [2] = LAYOUT_universal(
    KC_TAB   , KC_7     , KC_8     , KC_9     , KC_MINS  ,                            KC_NUHS  , _______  , KC_BTN3  , _______  , KC_BSPC  ,
   S(KC_QUOT), KC_4     , KC_5     , KC_6     ,S(KC_SCLN),                            S(KC_9)  , KC_BTN1  , KC_UP    , KC_BTN2  , KC_QUOT  ,
    KC_SLSH  , KC_1     , KC_2     , KC_3     ,S(KC_MINS),                           S(KC_NUHS), KC_LEFT  , KC_DOWN  , KC_RGHT  , _______  ,
    KC_ESC   , KC_0     , KC_DOT   , KC_DEL   , KC_ENT   , KC_BSPC  ,      _______  , _______  , _______  , _______  , _______  , _______
  ),

  [3] = LAYOUT_universal(
    RGB_TOG  , AML_TO   , AML_I50  , AML_D50  , _______  ,                            RGB_M_P  , RGB_M_B  , RGB_M_R  , RGB_M_SW , RGB_M_SN ,
    RGB_MOD  , RGB_HUI  , RGB_SAI  , RGB_VAI  , SCRL_DVI ,                            RGB_M_K  , RGB_M_X  , RGB_M_G  , RGB_M_T  , RGB_M_TW ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  , SCRL_DVD ,                            CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , KBC_SAVE ,
    QK_BOOT  , KBC_RST  , _______  , _______  , _______  , _______  ,      _______  , _______  , _______  , _______  , KBC_RST  , QK_BOOT
  ),
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
  [0] = { ENCODER_CCW_CW(KC_LEFT, KC_RGHT), ENCODER_CCW_CW(KC_UP  , KC_DOWN) },
  [1] = { ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______) },
  [2] = { ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______) },
  [3] = { ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______) },
};
#endif
// clang-format on

#ifdef RGBLIGHT_ENABLE
void keyboard_post_init_user(void) {
    // Boot default on both halves: white breathing, not saved to EEPROM.
    // Each half sets this itself; the slave does not wait for the L/R link.
    rgblight_enable_noeeprom();
    rgblight_sethsv_noeeprom(HSV_WHITE); // = layer 0 color
    rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING);
}
#endif

#ifdef RGBLIGHT_ENABLE
// Breathing color per layer (Japanese traditional colors, low saturation).
// Layer 4 (adjust) is not in the table: it is the mix of 2 (lower) and 3 (raise).
static const uint8_t PROGMEM layer_hs[][2] = {
    [0] = {0, 0},     // 月白   geppaku     : white       (normal)
    [1] = {113, 138}, // 薄荷   hakka       : choco-mint  (mouse / arrows)
    [2] = {145, 138}, // 勿忘草 wasurenagusa: sky blue    (lower: numbers)
    [3] = {245, 150}, // 鴇色   toki        : pale pink   (raise: symbols)
    [4] = {0, 0},     // 藤     fuji        : wisteria    (adjust) = mix of 2 and 3
    [5] = {28, 255},  // 山吹   yamabuki    : yellow, full saturation (tenkey)
};

static void set_layer_color(layer_state_t state) {
    uint8_t layer = get_highest_layer(state);
    if (layer >= sizeof(layer_hs) / sizeof(layer_hs[0])) layer = 0;
    uint8_t hue = pgm_read_byte(&layer_hs[layer][0]);
    uint8_t sat = pgm_read_byte(&layer_hs[layer][1]);
    if (layer == 4) {
        // Midpoint of lower and raise on the hue circle (shorter way round).
        uint8_t h2 = pgm_read_byte(&layer_hs[2][0]), h3 = pgm_read_byte(&layer_hs[3][0]);
        hue = h2 + (int8_t)(uint8_t)(h3 - h2) / 2;
        sat = (pgm_read_byte(&layer_hs[2][1]) + pgm_read_byte(&layer_hs[3][1])) / 2;
    }
    rgblight_sethsv_noeeprom(hue, sat, rgblight_get_val());
}
#endif

layer_state_t layer_state_set_user(layer_state_t state) {
    // Tri-layer: lower (2) + raise (3) = adjust (4). Works with any key that
    // turns on layers 2/3 (MO, LT, TD ...). Unlike update_tri_layer_state(),
    // only clear layer 4 if the tri-layer turned it on, so MO(4) still works.
    static bool tri_on = false;
    const layer_state_t lower_raise = ((layer_state_t)1 << 2) | ((layer_state_t)1 << 3);
    if ((state & lower_raise) == lower_raise) {
        state |= (layer_state_t)1 << 4;
        tri_on = true;
    } else if (tri_on) {
        state &= ~((layer_state_t)1 << 4);
        tri_on = false;
    }
#ifdef RGBLIGHT_ENABLE
    set_layer_color(state);
#endif
    return state;
}

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
