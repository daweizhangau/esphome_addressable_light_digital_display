#include "printable.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace addressable_light_digital_display {
static const char *const TAG = "text_display";

uint8_t Printable::print(uint8_t pos, const char *str) { return this->print_core(pos, str); }

uint8_t Printable::print(const char *str) { return this->print(0, str); }

uint8_t Printable::printf(uint8_t pos, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[64];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    return this->print(pos, buffer);
  return 0;
}

uint8_t Printable::printf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[64];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    return this->print(buffer);
  return 0;
}

uint8_t Printable::strftime(uint8_t pos, const char *format, ESPTime time) {
  char buffer[64];
  size_t ret = time.strftime(buffer, sizeof(buffer), format);
  if (ret > 0)
    return this->print(pos, buffer);
  return 0;
}

uint8_t Printable::strftime(const char *format, ESPTime time) { return this->strftime(0, format, time); }

}  // namespace addressable_light_digital_display
}  // namespace esphome
