# CLAUDE.md

ESPHome external component that turns an addressable LED strip into a 7-segment display (clocks, counters). Small hobby project: keep changes minimal, no frameworks, no CI.

## Layout

All code is in `components/addressable_light_digital_display/`:

- `display.py`: config schema and codegen. Validates `led_map` characters. Registers the object as both a display and a light.
- `digital_display.h/.cpp`: `DigitalDisplay`. Runs the user lambda every update, copies the LED on/off buffer to the internal addressable light in the colour of the external light. Print API for lambdas: `print(str)`, `printf(fmt, ...)`, `strftime(fmt, time)`.
- `segment_renderer.h`: `render_segments()`, the only real logic. Decides which LEDs are lit for a text and a `led_map`.
- `ascii_to_raw.h`: ASCII to segment bits table, bit order `XABCDEFG`.

`tests/test_segment_renderer.cpp` is the host test. `examples/digital_clock.yaml` is the user-facing example.

## Rules

- `segment_renderer.h` and `ascii_to_raw.h` must not include anything from ESPHome. They are compiled on a PC by the host test. That is why the table has no `PROGMEM`.
- Rendering changes go in `render_segments()` with a matching assert in the host test. Do not put rendering logic back into `DigitalDisplay`.
- `led_map` grammar: `A`-`G` segment, `X` per-character dot, `.` and `:` separator LEDs, space moves to the next character. `display.py` is the only place that rejects other characters; `render_segments()` relies on it.
- Separator rule: a `.`/`:` LED lights only when the current text char equals it; a space in the text occupies it dark; any other char leaves it dark and is not consumed.
- Characters with no 7-segment form light every segment. No logging from the renderer.
- There is no position argument on the print API. It was removed because it never worked; do not re-add it without implementing it.

## Verify

Host test (needs any `g++`, run from repo root):

```bash
g++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -Icomponents/addressable_light_digital_display tests/test_segment_renderer.cpp -o /tmp/test_segment_renderer && /tmp/test_segment_renderer
```

Prints `ok`.

ESPHome compile: write a throwaway config outside the repo that loads the component with `external_components: [{source: {type: local, path: <repo>/components}}]`, an addressable light, and a display lambda that calls `print`, `printf` and `strftime`, then run `esphome compile` on it (the `esphome/esphome` Docker image works). Expect `Successfully compiled program.` with no warnings from this component.

Last checked with ESPHome 2026.10.2 on ESP32 esp-idf with `esp32_rmt_led_strip`. Not checked: ESP8266, the arduino framework, `fastled_clockless`, real hardware.
