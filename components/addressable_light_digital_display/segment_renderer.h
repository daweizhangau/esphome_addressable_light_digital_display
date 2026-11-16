#pragma once

// No ESPHome includes here: this header is compiled on a PC by tests/test_segment_renderer.cpp.
#include <algorithm>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include "ascii_to_raw.h"

namespace esphome {
namespace addressable_light_digital_display {

/// Decide which LEDs are lit to show `text` on a strip wired as described by `led_map`.
///
/// led_map: one char per LED. 'A'-'G' = that segment of the current character, 'X' = its dot,
/// '.' / ':' = a separator LED, ' ' = move on to the next character of `text`.
/// A separator LED is lit only when the current text char equals it. A space in the text
/// occupies the separator dark. Any other text char leaves the separator dark and is kept
/// for the next digit, so "1234" and "12:34" both fit a map with a colon.
///
/// Characters with no 7-segment form light every segment. Text or led_map beyond the strip,
/// and LEDs beyond the text, are ignored (off).
inline void render_segments(const std::string &led_map, const char *text, bool reverse, bool *leds, size_t num_leds) {
  std::fill_n(leds, num_leds, false);

  const size_t text_len = strlen(text);
  size_t char_index = 0;
  size_t led_index = 0;
  bool separator_skipped = false;

  for (char map_char : led_map) {
    if (map_char == ' ') {
      if (!separator_skipped)
        char_index++;
      continue;
    }
    if (char_index >= text_len || led_index >= num_leds)
      break;

    const char c = text[reverse ? text_len - 1 - char_index : char_index];

    if (map_char == '.' || map_char == ':') {
      if (c != map_char && c != ' ') {
        separator_skipped = true;
        led_index++;
        continue;
      }
      leds[led_index] = c == map_char;
    } else {
      const uint8_t data = (c >= ' ' && c <= '~') ? ASCII_TO_RAW[c - ' '] : UNKNOWN_CHAR;
      // Bit order is XABCDEFG: X is bit 7, A bit 6 ... G bit 0.
      const uint8_t bit = map_char == 'X' ? 0x80 : 0x01 << ('G' - map_char);
      leds[led_index] = (data & bit) != 0;
    }

    led_index++;
    separator_skipped = false;
  }
}

}  // namespace addressable_light_digital_display
}  // namespace esphome
