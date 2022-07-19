#include "digital_display.h"
#include "ascii_to_raw.h"
#include "esphome/core/log.h"

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
  LOG_UPDATE_INTERVAL(this);
}

void DigitalDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up light display ...");
  this->num_leds_ = this->internal_light_output_->size();
}

void DigitalDisplay::update() {
  if (this->writer_.has_value())
    (*this->writer_)(*this);
  this->display();
}

void DigitalDisplay::display() {
  auto val = this->external_light_state_->current_values;
  auto max_brightness = to_uint8_scale(val.get_brightness() * val.get_state());
  auto color = color_from_light_color_values(val);

  // move to member to avoid recalculate length
  uint8_t map_length = this->led_map_.size();
  uint8_t char_index = 0;
  uint8_t led_index = 0;
  uint8_t bit_locator = 0;
  for (uint8_t i = 0; i < map_length; i++) {
    char map_char = this->led_map_[i];
    if (map_char == ' ') {
      char_index++;
      continue;
    }

    auto view = (*this->internal_light_output_)[led_index];
    if (map_char == 'X') {
      bit_locator = 0x80;
    } else {
      bit_locator = 0x01 << (6 - map_char + 'A');
    }
    ESP_LOGV(TAG, "Mapping char %c at %d, locator %X", map_char, i, bit_locator);

    // TODO: char length
    if ((bit_locator & this->buffer_[5 - 1 - char_index]) == bit_locator) {
      view = color;
    } else {
      view.set_rgbw(0, 0, 0, 0);
    }

    led_index++;
  }

  this->internal_light_output_->schedule_show();
}

uint8_t DigitalDisplay::print_core(uint8_t start_pos, const char *str) {
  // ESP_LOGV(TAG, "Print at %d: %s", start_pos, str);
  uint8_t pos = start_pos;
  for (; *str != '\0'; str++) {
    uint8_t data = UNKNOWN_CHAR;
    if (*str >= ' ' && *str <= '~')
      data = progmem_read_byte(&ASCII_TO_RAW[*str - ' ']);

    if (data == UNKNOWN_CHAR) {
      ESP_LOGW(TAG, "Encountered character '%c' with no representation while translating string!", *str);
    }

    // TODO: parameterize the limit based on LED configuration
    if (pos >= 6) {
      ESP_LOGE(TAG, "String is too long for the display!");
      break;
    }
    this->buffer_[pos] = data;
    pos++;
  }
  return pos - start_pos;
}

void DigitalDisplay::set_writer(writer_t &&writer) { this->writer_ = writer; }
void DigitalDisplay::set_led_map(const std::string &led_map) { this->led_map_ = led_map; }

void DigitalDisplay::set_internal_light(light::LightState *state) {
  this->internal_light_state_ = state;
  this->internal_light_output_ = static_cast<light::AddressableLight *>(state->get_output());
}

void DigitalDisplay::set_external_light(light::LightState *state) { this->external_light_state_ = state; }

}  // namespace addressable_light_digital_display
}  // namespace esphome
