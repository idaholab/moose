//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"

#define CONTROLLED_CONSOLE_TIMED_PRINT(initial_wait, dot_interval, ...)                            \
  mooseDeprecated("TIMED_PRINT is deprecated, use TIME_SECTION instead");

#define CONSOLE_TIMED_PRINT(...) CONTROLLED_CONSOLE_TIMED_PRINT(1, 1, __VA_ARGS__)

/// Dummy TimedPrint Class - use TIME_SECTION instead
class TimedPrint final
{
public:
  template <class StreamType, typename... Args>
  TimedPrint(StreamType &, std::chrono::duration<double>, std::chrono::duration<double>, Args &&...)
  {
    mooseDeprecated("TimedPrint is deprecated, use TIME_SECTION instead");
  }
};
