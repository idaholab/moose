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

namespace Moose::UnitUtils
{
/**
 * A helper for asserting that calling something throws an exception.
 *
 * @param action A function that calls the thing that should throw
 * @param contains Optional argument to check that the assertion message contains this sub string
 */
template <typename Action, class ExceptionType = std::exception>
void
assertThrows(const Action & action, const std::optional<std::string> & contains = {})
{
  try
  {
    action();
    FAIL() << "Expected " << MooseUtils::prettyCppType<ExceptionType>();
  }
  catch (ExceptionType const & e)
  {
    if (contains)
    {
      ASSERT_TRUE(std::string(e.what()).find(*contains) != std::string::npos)
          << "Exception \"" << e.what() << "\" does not contain \"" << *contains << "\"";
    }
  }
}
}
