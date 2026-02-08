//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StreamArguments.h"

#include <exception>
#include <string>

namespace Moose
{
/**
 * Common execption to be thrown when interacting with capabilities.
 */
class CapabilityException : public std::runtime_error
{
public:
  CapabilityException(const CapabilityException &) = default;

  template <typename... Args>
  static std::string stringify(Args &&... args)
  {
    std::ostringstream ss;
    streamArguments(ss, args...);
    return ss.str();
  }

  template <typename... Args>
  explicit CapabilityException(Args &&... args) : std::runtime_error(stringify(args...))
  {
  }

  ~CapabilityException() throw() {}
};
}
