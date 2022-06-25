#include "text_display.h"

namespace esphome {
namespace text_display{
static const char *const TAG = "text_display";

uint8_t TextDisplay::print(uint8_t pos, const char *str) { 
    return this->print_core(pos, str); 
}

uint8_t TextDisplay::print(const char* str) { 
    return this->print(0, str); 
}

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

uint8_t TextDisplay::strftime(const char* format, time::ESPTime time) { 
    return this->strftime(0, format, time); 
}
#endif

}  // namespace text_display
}  // namespace esphome
