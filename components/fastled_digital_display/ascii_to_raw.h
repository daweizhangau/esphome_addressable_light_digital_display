#pragma once

#include "esphome/core/hal.h"

namespace esphome {
namespace fastled_digital_display {

const uint8_t UNKNOWN_CHAR = 0b11111111;

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
    0b00000000,    // ' ', ord 0x20
    UNKNOWN_CHAR,  // '!', ord 0x21
    UNKNOWN_CHAR,  // '"', ord 0x22
    UNKNOWN_CHAR,  // '#', ord 0x23
    UNKNOWN_CHAR,  // '$', ord 0x24
    UNKNOWN_CHAR,  // '%', ord 0x25
    UNKNOWN_CHAR,  // '&', ord 0x26
    UNKNOWN_CHAR,  // ''', ord 0x27
    UNKNOWN_CHAR,  // '(', ord 0x28
    UNKNOWN_CHAR,  // ')', ord 0x29
    UNKNOWN_CHAR,  // '*', ord 0x2A
    UNKNOWN_CHAR,  // '+', ord 0x2B
    UNKNOWN_CHAR,  // ',', ord 0x2C
    UNKNOWN_CHAR,  // '-', ord 0x2D
    UNKNOWN_CHAR,  // '.', ord 0x2E
    UNKNOWN_CHAR,  // '/', ord 0x2F
    0b11111100,    // '0', ord 0x30
    0b10000100,    // '1', ord 0x31
    0b11011010,    // '2', ord 0x32
    0b11001110,    // '3', ord 0x33
    0b10100110,    // '4', ord 0x34
    0b01101110,    // '5', ord 0x35
    0b01111110,    // '6', ord 0x36
    0b11000100,    // '7', ord 0x37
    0b11111110,    // '8', ord 0x38
    0b11101110,    // '9', ord 0x39
    0b11000000,    // ':', ord 0x3A
};

}  // namespace fastled_digital_display
}  // namespace esphome
