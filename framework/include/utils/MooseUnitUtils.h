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

namespace Moose::UnitUtils::detail
{
template <typename T>
inline std::string
argToString(const T val)
{
  return std::to_string(val);
}
inline std::string
argToString(const char * val)
{
  return std::string(val);
}
inline std::string
argToString(const std::string & val)
{
  return val;
}
}

#define MOOSE_ASSERT_THROWS(exception_type, action, ...)                                           \
  do                                                                                               \
  {                                                                                                \
    static_assert(std::is_base_of_v<std::exception, exception_type>, "Not an exception");          \
    try                                                                                            \
    {                                                                                              \
      action;                                                                                      \
      FAIL() << "Expected " << MooseUtils::prettyCppType<exception_type>() << " not thrown";       \
    }                                                                                              \
    catch (std::exception const & e)                                                               \
    {                                                                                              \
      if constexpr (!std::is_same_v<std::exception, exception_type>)                               \
        if (!dynamic_cast<const exception_type *>(&e))                                             \
          FAIL() << "Threw " << MooseUtils::prettyCppType(&e) << " instead of "                    \
                 << MooseUtils::prettyCppType(&e) << "\n  Message: \"" << e.what() << "\"";        \
      if constexpr (sizeof(#__VA_ARGS__) > 1)                                                      \
      {                                                                                            \
        const auto contains_string = Moose::UnitUtils::detail::argToString(__VA_ARGS__);           \
        if (std::string(e.what()).find(contains_string) == std::string::npos)                      \
          FAIL() << "Exception " << MooseUtils::prettyCppType<exception_type>() << ":\n"           \
                 << "  Message: \"" << e.what() << "\"\n  Does not contain: \"" << contains_string \
                 << "\"";                                                                          \
      }                                                                                            \
    }                                                                                              \
  } while (0)

namespace Moose::UnitUtils
{
/**
 * A helper for asserting that calling something throws an exception.
 *
 * @param action A function that calls the thing that should throw
 * @param contains Optional argument to check that the assertion message contains this sub string
 */
template <class ExceptionType = std::exception, class Action = bool>
void
assertThrows(const Action & action, const std::optional<std::string> & contains = {})
{
  static_assert(std::is_base_of_v<std::exception, ExceptionType>, "Not an exception");

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
