#pragma once

#include <string>
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/components/light/addressable_light.h"
#include "printable.h"

namespace esphome {
namespace addressable_light_digital_display {
using namespace esphome::light;

class DigitalDisplay;
using writer_t = std::function<void(DigitalDisplay &)>;

class DigitalDisplay : public PollingComponent, public LightOutput, public Printable {
 public:
  // ========== PollingComponent =============
  void dump_config() override;
  void setup() override;
  void update() override;

  // ========== LightOutput =============
  light::LightTraits get_traits() override;
  void setup_state(LightState *state) override;
  void write_state(LightState *state) override;

  // ========== this =============
  void set_led_map(const std::string &led_map);
  void set_max_characters(const uint8_t max_characters);
  void set_reverse(const bool reverse);
  void set_internal_light(light::LightState *state);
  void set_external_light(light::LightState *state);
  void set_writer(writer_t &&writer);

 protected:
  void display();
  optional<writer_t> writer_{};
  uint8_t print_core(uint8_t pos, const char *str);
  bool *led_buffer_;
  std::string led_map_;
  uint8_t max_characters_;
  bool reverse_;

  int num_leds_{0};
  LightState *internal_light_state_;
  LightState *external_light_state_;
  AddressableLight *internal_light_output_;
};
}  // namespace addressable_light_digital_display
}  // namespace esphome
