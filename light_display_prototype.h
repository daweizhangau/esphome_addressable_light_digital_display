#pragma once

#include "esphome.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

#define DATA_PIN 33

class TextDisplay;

class TextDisplay {
 public:
  /// Evaluate the printf-format and print the result at the given position.
  uint8_t printf(uint8_t pos, const char *format, ...) __attribute__((format(printf, 3, 4)));
  /// Evaluate the printf-format and print the result at position 0.
  uint8_t printf(const char *format, ...) __attribute__((format(printf, 2, 3)));

  /// Print `str` at the given position.
  uint8_t print(uint8_t pos, const char *str);
  /// Print `str` at position 0.
  uint8_t print(const char *str);

#ifdef USE_TIME
  /// Evaluate the strftime-format and print the result at the given position.
  uint8_t strftime(uint8_t pos, const char *format, time::ESPTime time) __attribute__((format(strftime, 3, 0)));

  /// Evaluate the strftime-format and print the result at position 0.
  uint8_t strftime(const char *format, time::ESPTime time) __attribute__((format(strftime, 2, 0)));
#endif
 protected:
  virtual uint8_t print_core(uint8_t pos, const char *str) = 0;
};

uint8_t TextDisplay::print(uint8_t pos, const char *str) { return this->print_core(pos, str); }
uint8_t TextDisplay::print(const char* str) { return this->print(0, str); }

uint8_t TextDisplay::printf(uint8_t pos, const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[64];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    return this->print(pos, buffer);
  return 0;
}
uint8_t TextDisplay::printf(const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[64];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    return this->print(buffer);
  return 0;
}

#ifdef USE_TIME
uint8_t TextDisplay::strftime(uint8_t pos, const char* format, time::ESPTime time) {
  char buffer[64];
  size_t ret = time.strftime(buffer, sizeof(buffer), format);
  if (ret > 0)
    return this->print(pos, buffer);
  return 0;
}
uint8_t TextDisplay::strftime(const char* format, time::ESPTime time) { return this->strftime(0, format, time); }
#endif

class LightDigitDisplay;

using fastled_writer_t = std::function<void(LightDigitDisplay &)>;

class LightDigitDisplay : public TextDisplay, public PollingComponent {
 public:
  void dump_config() override;

  void setup() override;

  void set_writer(fastled_writer_t &&writer) { this->writer_ = writer; }

  void set_light_state(light::LightState *light_state) { this->light_state_ = light_state; } 
  void set_light_output(light::AddressableLight *light_output) { this->light_output_ = light_output; }

  void display_red() {  
    for (int i = 0; i < this->num_leds_; i++)
      this->leds_[i] = CRGB::Red;
  }

  void update() override;

  void display();

protected:
  uint8_t print_core(uint8_t pos, const char *str) override;
  optional<fastled_writer_t> writer_{};
  CRGB *leds_{nullptr};
  int num_leds_{0};
  light::AddressableLight *light_output_{nullptr};
  light::LightState *light_state_{nullptr};
  uint8_t buffer_[5] = {0};
};

static const char *TAG = "light-display";
const uint8_t UNKNOWN_CHAR = 0b11111111;

static std::string SEGS =     "77277"; // number of bits by character
static std::string SEG_LEDS = "111111111111111111111111111111"; // number of leds by bit

void LightDigitDisplay::display() {
  float red, green, blue;
  this->light_state_->current_values_as_rgb(&red, &green, &blue);

  int led_index = 0;
  int bit_index = 0;
  int char_length = SEGS.size();
  for (int data_offset = 0; data_offset < SEGS.length(); data_offset++) 
  {
    uint8_t data = this->buffer_[char_length - 1 - data_offset];
    int data_bits = SEGS[data_offset] - '0';
    //ESP_LOGD(TAG, "data_offset=%d, bits=%d", data_offset, data_bits);
    for(int bit_offset = 0; bit_offset < data_bits; bit_offset++)
    {
      bool on = (data << bit_offset) & 0b10000000;
      //auto color = on ? CRGB::Red : CRGB::Black;
      int bit_leds = SEG_LEDS[bit_index] - '0';
      //ESP_LOGD(TAG, "bit_leds=%d, on=%s", bit_leds, on ? "true" : "false");
      for(int led_offset = 0; led_offset < bit_leds; led_offset++)
      {
        //this->leds_[led_index] = color;
        auto view = this->light_output_->get(led_index);
        view.set_red(on && red > 0 ? 255 : 0);
        view.set_green(on && green > 0 ? 255 : 0);
        view.set_blue(on && blue > 0 ? 255 : 0);
        //ESP_LOGD(TAG, "red_raw=%d,red=%d, on=%s", view.get_red_raw(), view.get_red(), on ? "true" : "false");
        
        led_index++;
      }

      bit_index++;
    }
  }

  this->light_output_->schedule_show();
}

//
//      B
//     ---
//  C |   | A
//     -G-
//  D |   | F
//     ---
//      E   
// ABCDEFG
const uint8_t ASCII_TO_RAW[] PROGMEM = {
    0b00000000,           // ' ', ord 0x20
    UNKNOWN_CHAR,           // '!', ord 0x21
    UNKNOWN_CHAR,           // '"', ord 0x22
    UNKNOWN_CHAR,  // '#', ord 0x23
    UNKNOWN_CHAR,  // '$', ord 0x24
    UNKNOWN_CHAR,           // '%', ord 0x25
    UNKNOWN_CHAR,  // '&', ord 0x26
    UNKNOWN_CHAR,           // ''', ord 0x27
    UNKNOWN_CHAR,           // '(', ord 0x28
    UNKNOWN_CHAR,           // ')', ord 0x29
    UNKNOWN_CHAR,           // '*', ord 0x2A
    UNKNOWN_CHAR,  // '+', ord 0x2B
    UNKNOWN_CHAR,           // ',', ord 0x2C
    UNKNOWN_CHAR,           // '-', ord 0x2D
    UNKNOWN_CHAR,           // '.', ord 0x2E
    UNKNOWN_CHAR,  // '/', ord 0x2F
    0b11111100,           // '0', ord 0x30
    0b10000100,           // '1', ord 0x31
    0b11011010,           // '2', ord 0x32
    0b11001110,           // '3', ord 0x33
    0b10100110,           // '4', ord 0x34
    0b01101110,           // '5', ord 0x35
    0b01111110,           // '6', ord 0x36
    0b11000100,           // '7', ord 0x37
    0b11111110,           // '8', ord 0x38
    0b11101110,           // '9', ord 0x39
    0b11000000,           // ':', ord 0x3A
};

uint8_t LightDigitDisplay::print_core(uint8_t start_pos, const char* str) {
  ESP_LOGV(TAG, "Print at %d: %s", start_pos, str);
  uint8_t pos = start_pos;
  for (; *str != '\0'; str++) {
    uint8_t data = UNKNOWN_CHAR;
    if (*str >= ' ' && *str <= '~')
      data = pgm_read_byte(&ASCII_TO_RAW[*str - ' ']);

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

void LightDigitDisplay::dump_config() {
  ESP_LOGCONFIG(TAG, "Light display");
  ESP_LOGCONFIG(TAG, "  Update interval (ms): %d", this->get_update_interval());
  ESP_LOGCONFIG(TAG, "  Number of LEDs: %d", this->num_leds_);
}

void LightDigitDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up light display ...");
  this->num_leds_ = this->light_output_->size();
}

void LightDigitDisplay::update() {
  if (this->writer_.has_value())
    (*this->writer_)(*this);
  this->display();
}
