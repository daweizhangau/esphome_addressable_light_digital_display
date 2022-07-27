#include "digital_display.h"
#include "ascii_to_raw.h"
#include "esphome/core/log.h"
#include <stdio.h>
#include <string.h>

namespace esphome {
namespace addressable_light_digital_display {

static const char *const TAG = "addressable_light_digital_display.digital_display";

light::LightTraits DigitalDisplay::get_traits() { return this->internal_light_output_->get_traits(); }

void DigitalDisplay::setup_state(light::LightState *state) {
  ESP_LOGD(TAG, "Setup state");
  this->external_light_state_ = state;
}

void DigitalDisplay::write_state(light::LightState *state) {
  ESP_LOGD(TAG, "Writing state");
  this->internal_light_output_->update_state(state);
  this->display();
}

void DigitalDisplay::dump_config() {
  ESP_LOGCONFIG(TAG, "Light display");
  ESP_LOGCONFIG(TAG, "  Update interval (ms): %d", this->get_update_interval());
  ESP_LOGCONFIG(TAG, "  Number of LEDs: %d", this->num_leds_);
  ESP_LOGCONFIG(TAG, "  LED MAP: %s", (this->led_map_).c_str());
  ESP_LOGCONFIG(TAG, "  Max characters: %d", this->max_characters_);
  ESP_LOGCONFIG(TAG, "  Reverse: %s", this->reverse_ ? "True" : "False");
  LOG_UPDATE_INTERVAL(this);
}

void DigitalDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up light display ...");
  this->num_leds_ = this->internal_light_output_->size();
  this->led_buffer_ = new bool[this->num_leds_];
}

void DigitalDisplay::update() {
  if (this->writer_.has_value())
    (*this->writer_)(*this);
  this->display();
}

void DigitalDisplay::display() {
  ESP_LOGV(TAG, "display()");
  auto val = this->external_light_state_->current_values;
  auto color = color_from_light_color_values(val);

  for (uint8_t i = 0; i < this->num_leds_; i++) {
    light::ESPColorView view = (*this->internal_light_output_)[i];
    if (this->led_buffer_[i]) {
      view.set(color);
    } else {
      view.set_rgbw(0, 0, 0, 0);
    }
  }

  this->internal_light_output_->schedule_show();
}

uint8_t DigitalDisplay::print_core(uint8_t start_pos, const char *str) {
  if (start_pos != 0) {
    ESP_LOGE(TAG, "Only Position 0 is supported.");
  }

  ESP_LOGV(TAG, "Print at %d: %s", start_pos, str);
  uint8_t pos = start_pos;
  uint8_t data = UNKNOWN_CHAR;

  uint8_t map_length = this->led_map_.size();
  uint8_t char_index = 0;
  uint8_t led_index = 0;
  uint8_t bit_locator = 0;
  char current_char = ' ';
  uint8_t max_char = strlen(str);
  uint8_t max_led = this->num_leds_;
  bool reverse = this->reverse_;
  char current_map_char;
  bool led_on = false;
  bool is_special_map_char = false;
  bool skip_led = false;

  for (uint8_t i = 0; i < max_led; i++) {
    this->led_buffer_[i] = false;
  }

  for (uint8_t map_index = 0; map_index < map_length; map_index++) {
    current_map_char = this->led_map_[map_index];
    current_char = reverse ? str[max_char - 1 - char_index] : str[char_index];
    is_special_map_char = current_map_char == '.' || current_map_char == ':';
    if (is_special_map_char && (current_char != current_map_char) && (current_char != ' ')){
      skip_led = true;
      led_index++;
      continue;
    }

    if (current_map_char == ' ') {
      if (! skip_led){char_index++;}
      continue;
    }

    if (char_index >= max_char) { break; }

    if (is_special_map_char) {
      led_on = current_char == current_map_char;
    } else {
      if (*str >= ' ' && *str <= '~') {
        data = progmem_read_byte(&ASCII_TO_RAW[current_char - ' ']);
      }

      if (data == UNKNOWN_CHAR) {
        ESP_LOGW(TAG, "Encountered character '%c' with no representation while translating string!", *str);
      }

      bit_locator = 0x01 << (6 - current_map_char + 'A');
      led_on = (bit_locator & data) == bit_locator;
    }

    this->led_buffer_[led_index] = led_on;
    led_index++;
    if (led_index >= max_led) { break; }
    skip_led = false;
  }

  return 0;
}

void DigitalDisplay::set_writer(writer_t &&writer) { this->writer_ = writer; }
void DigitalDisplay::set_led_map(const std::string &led_map) { this->led_map_ = led_map; }
void DigitalDisplay::set_max_characters(const uint8_t max_characters) { this->max_characters_ = max_characters; }
void DigitalDisplay::set_reverse(const bool reverse) { this->reverse_ = reverse; }

void DigitalDisplay::set_internal_light(light::LightState *state) {
  this->internal_light_state_ = state;
  this->internal_light_output_ = static_cast<light::AddressableLight *>(state->get_output());
}

void DigitalDisplay::set_external_light(light::LightState *state) { this->external_light_state_ = state; }

}  // namespace addressable_light_digital_display
}  // namespace esphome
