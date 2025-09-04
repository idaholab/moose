//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest/gtest.h"

#include "MooseUtils.h"
#include "Moose.h"

namespace Moose::UnitUtils
{
/**
 * A helper for asserting that calling something throws an exception.
 *
 * @param action A function that calls the thing that should throw
 * @param contains Optional argument to check that the assertion message contains this sub string
 * @param set_throw_on_error Set to true to set moose to throw on error
 */
template <class ExceptionType = std::exception, class Action = bool>
void
assertThrows(const Action & action,
             const std::optional<std::string> & contains = {},
             const bool set_throw_on_error = false)
{
  static_assert(std::is_base_of_v<std::exception, ExceptionType>, "Not an exception");

  std::unique_ptr<Moose::ScopedThrowOnError> scoped_throw_on_error;
  if (set_throw_on_error)
    scoped_throw_on_error = std::make_unique<Moose::ScopedThrowOnError>();

  try
  {
    action();
    FAIL() << "Expected " << MooseUtils::prettyCppType<ExceptionType>() << " not thrown";
  }
  catch (std::exception const & e)
  {
    if constexpr (!std::is_same_v<std::exception, ExceptionType>)
      if (!dynamic_cast<const ExceptionType *>(&e))
        FAIL() << "Threw " << demangle(typeid(e).name()) << " instead of "
               << MooseUtils::prettyCppType<ExceptionType>() << " with message '" << e.what()
               << "'";

    if (contains)
    {
      ASSERT_TRUE(std::string(e.what()).find(*contains) != std::string::npos)
          << "Exception \"" << e.what() << "\" does not contain \"" << *contains << "\"";
    }
  }
}
}
