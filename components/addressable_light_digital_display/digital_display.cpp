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

  int led_index = 0;
  int bit_index = 0;
  auto partitions = this->char_segments_;
  int char_length = partitions.size();
  for (int data_offset = 0; data_offset < partitions.length(); data_offset++) {
    uint8_t data = this->buffer_[char_length - 1 - data_offset];
    int data_bits = partitions[data_offset] - '0';
    for (int bit_offset = 0; bit_offset < data_bits; bit_offset++) {
      bool on = (data << bit_offset) & 0b10000000;
      int bit_leds = this->segment_leds_[bit_index] - '0';
      for (int led_offset = 0; led_offset < bit_leds; led_offset++) {
        light::ESPColorView view = (*this->internal_light_output_)[led_index];
        if (on) {
          view = color;
        } else {
          view.set_red(0);
          view.set_green(0);
          view.set_blue(0);
        }

        led_index++;
      }

      bit_index++;
    }
  }

  this->internal_light_output_->schedule_show();
}

uint8_t DigitalDisplay::print_core(uint8_t start_pos, const char *str) {
  ESP_LOGV(TAG, "Print at %d: %s", start_pos, str);
  uint8_t pos = start_pos;
  for (; *str != '\0'; str++) {
    uint8_t data = UNKNOWN_CHAR;
    if (*str >= ' ' && *str <= '~')
      data = progmem_read_byte(&ASCII_TO_RAW[*str - ' ']);

    if (data == UNKNOWN_CHAR) {
      ESP_LOGW(TAG, "Encountered character '%c' with no representation while translating string!", *str);
    }

    if (*str == '.') {
      if (pos != start_pos)
        pos--;
      this->buffer_[pos] |= 0b10000000;
    } else {
      if (pos >= 5) {
        ESP_LOGE(TAG, "String is too long for the display!");
        break;
      }
      this->buffer_[pos] = data;
    }
    pos++;
  }
  return pos - start_pos;
}

void DigitalDisplay::set_writer(writer_t &&writer) { this->writer_ = writer; }
void DigitalDisplay::set_segments(const std::string &segments) { this->char_segments_ = segments; }
void DigitalDisplay::set_segment_leds(const std::string &segment_leds) { this->segment_leds_ = segment_leds; }

void DigitalDisplay::set_internal_light(light::LightState *state) {
  this->internal_light_state_ = state;
  this->internal_light_output_ = static_cast<light::AddressableLight *>(state->get_output());
}

void DigitalDisplay::set_external_light(light::LightState *state) { this->external_light_state_ = state; }

}  // namespace addressable_light_digital_display
}  // namespace esphome
