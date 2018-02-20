#ifndef __TRMCLR_H__
#define __TRMCLR_H__

// Copyright (c) 2016 Camille Brugel
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <cstdint>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>

// ## Example

// ```c++
// #include "trmclr.hpp"
// #include <iostream>

// int main()
// {
//     trmclr::Style fancyStyle(trmclr::Background::LIGHT_BLUE    |
//                              trmclr::Foreground::WHITE         |
//                              trmclr::Attribute::UNDERLINED     |
//                              trmclr::Attribute::BOLD);

//     trmclr::Style basicStyle(trmclr::Attribute::DEFAULT);

//     std::cout << fancyStyle << "Hello "
//               << basicStyle << "World!\n";

//     return 0;
// }

// /*
// Note you can also do things like:
// auto bold = [](trmclr::Style style) { return trmclr::Style(style | trmclr::Attribute::BOLD); };
// */
// `

namespace trmclr
{

struct Style
{
  Style(uint32_t value) : _value(value) {}

  operator uint32_t() const { return _value; }

  uint32_t _value;
};

enum StyleTypes
{
  FOREGROUND,
  ATTRIBUTE,
  BACKGROUND,
  N_STYLE_TYPES
};

static const uint32_t STYLE_SHIFT = std::numeric_limits<uint32_t>::digits / N_STYLE_TYPES;

struct Attribute
{
  static const uint32_t SHIFT = STYLE_SHIFT * ATTRIBUTE;

  enum
  {
    DEFAULT = 0x01 << SHIFT,
    BOLD = 0x02 << SHIFT,
    DIM = 0x04 << SHIFT,
    UNDERLINED = 0x08 << SHIFT,
    BLINK = 0x10 << SHIFT,
    REVERSE = 0x20 << SHIFT,
    HIDDEN = 0x40 << SHIFT
  };
};

struct Foreground
{
  static const uint32_t SHIFT = STYLE_SHIFT * FOREGROUND;

  enum
  {
    BLACK = 30 << SHIFT,
    RED = 31 << SHIFT,
    GREEN = 32 << SHIFT,
    YELLOW = 33 << SHIFT,
    BLUE = 34 << SHIFT,
    MAGENTA = 35 << SHIFT,
    CYAN = 36 << SHIFT,
    LIGHT_GRAY = 37 << SHIFT,
    DEFAULT = 39 << SHIFT,
    DARK_GRAY = 90 << SHIFT,
    LIGHT_RED = 91 << SHIFT,
    LIGHT_GREEN = 92 << SHIFT,
    LIGHT_YELLOW = 93 << SHIFT,
    LIGHT_BLUE = 94 << SHIFT,
    LIGHT_MAGENTA = 95 << SHIFT,
    LIGHT_CYAN = 96 << SHIFT,
    WHITE = 97 << SHIFT
  };
};

struct Background
{
  static const uint32_t SHIFT = STYLE_SHIFT * BACKGROUND;

  enum
  {
    BLACK = 40 << SHIFT,
    RED = 41 << SHIFT,
    GREEN = 42 << SHIFT,
    YELLOW = 43 << SHIFT,
    BLUE = 44 << SHIFT,
    MAGENTA = 45 << SHIFT,
    CYAN = 46 << SHIFT,
    LIGHT_GRAY = 47 << SHIFT,
    DEFAULT = 49 << SHIFT,
    DARK_GRAY = 100 << SHIFT,
    LIGHT_RED = 101 << SHIFT,
    LIGHT_GREEN = 102 << SHIFT,
    LIGHT_YELLOW = 103 << SHIFT,
    LIGHT_BLUE = 104 << SHIFT,
    LIGHT_MAGENTA = 105 << SHIFT,
    LIGHT_CYAN = 106 << SHIFT,
    WHITE = 107 << SHIFT
  };
};

static Style black(trmclr::Foreground::BLACK);
static Style red(trmclr::Foreground::RED);
static Style green(trmclr::Foreground::GREEN);
static Style yellow(trmclr::Foreground::YELLOW);
static Style blue(trmclr::Foreground::BLUE);
static Style magenta(trmclr::Foreground::MAGENTA);
static Style cyan(trmclr::Foreground::CYAN);
static Style normal(trmclr::Attribute::DEFAULT);

inline std::ostream &
operator<<(std::ostream & os, const Style & style)
{
  const uint32_t base = 1 << STYLE_SHIFT;
  uint32_t encoded = style / base;
  uint32_t decoded = style % base;

  os << "\x1B[" << (decoded ? decoded : Foreground::DEFAULT >> Foreground::SHIFT);

  decoded = encoded % base;

  for (uint32_t i = 0; decoded != 0; decoded >>= 1, i++)
  {
    if (decoded & 1)
    {
      os << ";" << i;
    }
  }

  encoded = encoded / base;
  decoded = encoded % base;

  os << ";" << (decoded ? decoded : Background::DEFAULT >> Background::SHIFT) << "m";

  return os;
}

} // namespace trmclr
#endif // end __TRMCLR_H__
