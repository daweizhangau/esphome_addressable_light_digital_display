#pragma once

#include <string>
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/light/light_output.h"

#include "text_display.h"

namespace esphome {
namespace fastled_digital_display {
using namespace esphome::light;

class FastLEDDigitalDisplay;
using writer_t = std::function<void(FastLEDDigitalDisplay &)>;

class FastLEDDigitalDisplay : public TextDisplay, public PollingComponent {
 public:
  void set_writer(writer_t &&writer);
  void set_segments(const std::string &segments);
  void set_segment_leds(const std::string &segment_leds);
  void dump_config() override;

  void set_light_state(LightState *light_state);
  void set_light_output(AddressableLight *light_output);

  void setup() override;
  void update() override;
  void display();

 protected:
  optional<writer_t> writer_{};
  uint8_t print_core(uint8_t pos, const char *str);
  uint8_t buffer_[5] = {0};
  std::string partitions_;
  std::string partition_leds_;

  int num_leds_{0};
  LightState *light_state_{nullptr};
  AddressableLight *light_output_{nullptr};
};
}  // namespace fastled_digital_display
}  // namespace esphome
