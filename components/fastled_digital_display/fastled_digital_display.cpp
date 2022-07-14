#include "fastled_digital_display.h"
#include "ascii_to_raw.h"
#include "esphome/core/log.h"

namespace esphome {
namespace fastled_digital_display {

static const char *const TAG = "display.fastled_digital_display";

void FastLEDDigitalDisplay::dump_config() {
  ESP_LOGCONFIG(TAG, "Light display");
  ESP_LOGCONFIG(TAG, "  Update interval (ms): %d", this->get_update_interval());
  ESP_LOGCONFIG(TAG, "  Number of LEDs: %d", this->num_leds_);
  LOG_UPDATE_INTERVAL(this);
}

void FastLEDDigitalDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up light display ...");
  this->num_leds_ = this->light_output_->size();
}

void FastLEDDigitalDisplay::update() {
  if (this->writer_.has_value())
    (*this->writer_)(*this);
  this->display();
}

void FastLEDDigitalDisplay::display() {
  float red, green, blue;
  this->light_state_->current_values_as_rgb(&red, &green, &blue);

  int led_index = 0;
  int bit_index = 0;
  auto partitions = this->partitions_;
  int char_length = partitions.size();
  for (int data_offset = 0; data_offset < partitions.length(); data_offset++) {
    uint8_t data = this->buffer_[char_length - 1 - data_offset];
    int data_bits = partitions[data_offset] - '0';
    // ESP_LOGD(TAG, "data_offset=%d, bits=%d", data_offset, data_bits);
    for (int bit_offset = 0; bit_offset < data_bits; bit_offset++) {
      bool on = (data << bit_offset) & 0b10000000;
      // auto color = on ? CRGB::Red : CRGB::Black;
      int bit_leds = this->partition_leds_[bit_index] - '0';
      // ESP_LOGD(TAG, "bit_leds=%d, on=%s", bit_leds, on ? "true" : "false");
      for (int led_offset = 0; led_offset < bit_leds; led_offset++) {
        // this->leds_[led_index] = color;
        auto view = this->light_output_->get(led_index);
        view.set_red(on && red > 0 ? 255 : 0);
        view.set_green(on && green > 0 ? 255 : 0);
        view.set_blue(on && blue > 0 ? 255 : 0);
        // ESP_LOGD(TAG, "red_raw=%d,red=%d, on=%s", view.get_red_raw(), view.get_red(), on ? "true" : "false");

        led_index++;
      }

      bit_index++;
    }
  }

  this->light_output_->schedule_show();
}

uint8_t FastLEDDigitalDisplay::print_core(uint8_t start_pos, const char *str) {
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

void FastLEDDigitalDisplay::set_writer(writer_t &&writer) { this->writer_ = writer; }
void FastLEDDigitalDisplay::set_segments(const std::string &segments) { this->partitions_ = segments; }
void FastLEDDigitalDisplay::set_segment_leds(const std::string &segment_leds) { this->partition_leds_ = segment_leds; }

void FastLEDDigitalDisplay::set_light_state(LightState *light_state) { this->light_state_ = light_state; }
void FastLEDDigitalDisplay::set_light_output(AddressableLight *light_output) { this->light_output_ = light_output; }
}  // namespace fastled_digital_display
}  // namespace esphome
