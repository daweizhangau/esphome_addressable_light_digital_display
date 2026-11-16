#pragma once

#include <string>
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/time.h"
#include "esphome/components/light/addressable_light.h"

namespace esphome {
namespace addressable_light_digital_display {
using namespace esphome::light;

class DigitalDisplay;
using writer_t = std::function<void(DigitalDisplay &)>;

class DigitalDisplay : public PollingComponent, public LightOutput {
 public:
  // ========== PollingComponent =============
  void dump_config() override;
  void setup() override;
  void update() override;

  // ========== LightOutput =============
  light::LightTraits get_traits() override;
  void setup_state(LightState *state) override;
  void write_state(LightState *state) override;

  // ========== Print API, for use in the display lambda =============
  /// Show `str`, replacing whatever was shown before.
  void print(const char *str);
  /// Evaluate the printf-format and show the result.
  void printf(const char *format, ...) __attribute__((format(printf, 2, 3)));
  /// Evaluate the strftime-format and show the result.
  void strftime(const char *format, ESPTime time) __attribute__((format(strftime, 2, 0)));

  // ========== Configuration, called from generated code =============
  void set_led_map(const std::string &led_map);
  void set_reverse(const bool reverse);
  void set_internal_light(light::LightState *state);
  void set_writer(writer_t &&writer);

 protected:
  void display();
  optional<writer_t> writer_{};
  bool *led_buffer_;
  std::string led_map_;
  bool reverse_;

  int num_leds_{0};
  LightState *external_light_state_;
  AddressableLight *internal_light_output_;
};
}  // namespace addressable_light_digital_display
}  // namespace esphome
