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

#include <stdexcept>
#include <sstream>

namespace moose
{
namespace internal
{
template <typename... Args>
std::string
streamArgsToString(Args &&... args)
{
  std::ostringstream ss;
  streamArguments(ss, args...);
  return ss.str();
}
}
}

/**
 * Provides a way for users to bail out of the current solve.
 */
class MooseException : public std::runtime_error
{
public:
  /**
   * @param message The message to display
   */
  MooseException(const std::string & message) : std::runtime_error(message) {}

  /**
   * Set an explicit default constructor to avoid the variadic template constructor
   * below catch the copy construction case.
   */
  MooseException(const MooseException &) = default;

  /**
   * @param args List of arguments that gets stringified and concatenated to form the message to
   * display
   */
  template <typename... Args>
  explicit MooseException(Args &&... args)
    : std::runtime_error(moose::internal::streamArgsToString(std::forward<Args>(args)...))
  {
  }
};
