#include "digital_display.h"
#include "segment_renderer.h"
#include "esphome/core/log.h"
#include <stdarg.h>
#include <stdio.h>

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
  ESP_LOGCONFIG(TAG, "  Number of LEDs: %d", this->num_leds_);
  ESP_LOGCONFIG(TAG, "  LED MAP: %s", (this->led_map_).c_str());
  ESP_LOGCONFIG(TAG, "  Reverse: %s", this->reverse_ ? "True" : "False");
  LOG_UPDATE_INTERVAL(this);
}

void DigitalDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up light display ...");
  this->num_leds_ = this->internal_light_output_->size();
  this->led_buffer_ = new bool[this->num_leds_]();
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

  for (int i = 0; i < this->num_leds_; i++) {
    light::ESPColorView view = (*this->internal_light_output_)[i];
    if (this->led_buffer_[i]) {
      view.set(color);
    } else {
      view.set_rgbw(0, 0, 0, 0);
    }
  }

  this->internal_light_output_->schedule_show();
}

void DigitalDisplay::print(const char *str) {
  ESP_LOGV(TAG, "Print: %s", str);
  render_segments(this->led_map_, str, this->reverse_, this->led_buffer_, this->num_leds_);
}

void DigitalDisplay::printf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[64];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    this->print(buffer);
}

void DigitalDisplay::strftime(const char *format, ESPTime time) {
  char buffer[64];
  if (time.strftime(buffer, sizeof(buffer), format) > 0)
    this->print(buffer);
}

void DigitalDisplay::set_writer(writer_t &&writer) { this->writer_ = writer; }
void DigitalDisplay::set_led_map(const std::string &led_map) { this->led_map_ = led_map; }
void DigitalDisplay::set_reverse(const bool reverse) { this->reverse_ = reverse; }

void DigitalDisplay::set_internal_light(light::LightState *state) {
  this->internal_light_output_ = static_cast<light::AddressableLight *>(state->get_output());
}

}  // namespace addressable_light_digital_display
}  // namespace esphome
