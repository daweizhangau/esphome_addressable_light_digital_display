# ESPHome Component: FastLED Digital Display

This component enables ESPHome to drive an eight-segment display created by FastLED strip.

As it's an ESPHome component, it can be used for different purposes. For example, it can display date, time, room temperature, noise level etc. Data can be pulled from Home Assistant.

Comparing to LED Matrix and traditional eight-segment LED display, this display has unique advantages. 
  - It is not constrained by display size or DPI. You can create a large sharp digital display with minimal number of LEDs. It costs much less than using large eight-segment LED modules. 
  - The display can be extended easily.  Displaying more digits doesn't increase circuit complexity. Most FastLED can be extended with 3 to 4 wires only. No additional ICs or ports required if the power supply can cope with the load.
  - Brightness and colour can be easily controlled by software.
