// Host test for the segment renderer. No ESPHome needed. Run from the repo root:
//   g++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -Icomponents/addressable_light_digital_display tests/test_segment_renderer.cpp -o /tmp/test_segment_renderer && /tmp/test_segment_renderer
#include <assert.h>
#include <stdio.h>
#include <string>
#include "segment_renderer.h"

using esphome::addressable_light_digital_display::render_segments;

// Render onto a strip of `n` LEDs and return it as a string of '1' / '0'.
// Sanitizers catch any write past the strip.
static std::string render(const std::string &led_map, const char *text, bool reverse, size_t n) {
  bool *leds = new bool[n];
  render_segments(led_map, text, reverse, leds, n);
  std::string out;
  for (size_t i = 0; i < n; i++)
    out += leds[i] ? '1' : '0';
  delete[] leds;
  return out;
}

int main() {
  const std::string digit = "ABCDEFG";
  const std::string clock = "ABCDEFG ABCDEFG : ABCDEFG ABCDEFG";  // 29 LEDs

  // Single characters, segment order ABCDEFG.
  assert(render(digit, "8", false, 7) == "1111111");
  assert(render(digit, "1", false, 7) == "0110000");
  assert(render(digit, " ", false, 7) == "0000000");
  // Segment letters may come in any order and repeat (several LEDs per segment).
  assert(render("GGBA", "1", false, 4) == "0010");

  // 'X' is the per-character dot (bit 7).
  assert(render("X", ".", false, 1) == "1");
  assert(render("X", "1", false, 1) == "0");

  // Separator: lit on match, dark for a space, skipped when the text has none.
  assert(render(clock, "11:11", false, 29) == "0110000" "0110000" "1" "0110000" "0110000");
  assert(render(clock, "11 11", false, 29) == "0110000" "0110000" "0" "0110000" "0110000");
  assert(render(clock, "1111", false, 29) == "0110000" "0110000" "0" "0110000" "0110000");

  // Reverse: first LED belongs to the last character.
  assert(render("ABCDEFG ABCDEFG", "18", true, 14) == "1111111" "0110000");
  assert(render("ABCDEFG ABCDEFG", "18", false, 14) == "0110000" "1111111");

  // Text shorter than the map: the rest stays off. Text longer: extra ignored.
  assert(render(clock, "1", false, 29) == "0110000" + std::string(22, '0'));
  assert(render(digit, "18", false, 7) == "0110000");

  // Empty text, both directions (reverse used to read text[-1]).
  assert(render(clock, "", true, 29) == std::string(29, '0'));
  assert(render(clock, "", false, 29) == std::string(29, '0'));

  // No 7-segment form, including non-ASCII after a valid first char: all segments lit.
  assert(render(digit, "#", false, 7) == "1111111");
  assert(render("ABCDEFG ABCDEFG", "1\xC3", false, 14) == "0110000" "1111111");

  // led_map longer than the strip, on both the digit path and the skipped-separator path.
  assert(render(clock, "88:88", false, 3) == "111");
  assert(render(":::::", "1", false, 2) == "00");
  assert(render(digit, "8", false, 0) == "");

  // More than 255 LEDs / map chars (counters used to be uint8_t).
  std::string big;
  for (int i = 0; i < 50; i++)
    big += "ABCDEFG ";
  assert(render(big, std::string(50, '8').c_str(), false, 350) == std::string(350, '1'));

  puts("ok");
  return 0;
}
