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

#include <filesystem>
#include <string>

#include "libmesh/utility.h"

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

/**
 * Helper for the [EXPECT,ASSERT]_[MOOSEERROR,THROW]_[MSG,MSG_CONTAINS] macros.
 */
template <typename ExceptionType, bool exact, bool assert, bool set_throw_on_error, typename Func>
void
throwsWithMessage(Func && fn, const std::string_view message, const char * file, int line)
{
  static_assert(std::is_base_of_v<std::exception, ExceptionType>, "Not an exception");

  std::ostringstream error;

  try
  {
    std::unique_ptr<Moose::ScopedThrowOnError> scoped_throw_on_error;
    if constexpr (set_throw_on_error)
      scoped_throw_on_error = std::make_unique<Moose::ScopedThrowOnError>();

    fn();

    error << "Expected exception of type " << libMesh::demangle(typeid(ExceptionType).name())
          << " but no exception was thrown.";
  }
  catch (const ExceptionType & ex)
  {
    // Exact match
    if constexpr (exact)
    {
      if (std::string(ex.what()) == message)
        return;
    }
    // Partial match
    else
    {
      if (std::string(ex.what()).find(message) != std::string::npos)
        return;
    }

    error << "Expected" << (exact ? "" : " partial") << " exception message: \"" << message
          << "\"\n  Actual exception message: \"" << ex.what() << "\"";
  }
  catch (const std::exception & ex)
  {
    error << "Expected exception of type " << libMesh::demangle(typeid(ExceptionType).name())
          << " but exception of type " << libMesh::demangle(typeid(ex).name()) << " was thrown.";
  }

  ::testing::internal::AssertHelper(
      ::testing::TestPartResult::kNonFatalFailure, file, line, error.str().c_str()) =
      ::testing::Message();
}

/**
 * Create a temporary file and delete it upon destruction.
 */
class TempFile
{
public:
  TempFile();
  ~TempFile();

  /**
   * @return The path to the temporary file.
   */
  const std::filesystem::path & path() const { return _path; }

private:
  static std::filesystem::path generatePath();

  const std::filesystem::path _path;
};

} // namespace Moose::UnitUtils

/// Expect that an action throws with an exact message
#define EXPECT_THROW_MSG(stmt, exc_type, expected_msg)                                             \
  ::Moose::UnitUtils::throwsWithMessage<exc_type, true, false, false>(                             \
      [&]() { stmt; }, expected_msg, __FILE__, __LINE__)

/// Assert that an action throws with an exact message
#define ASSERT_THROW_MSG(stmt, exc_type, expected_msg)                                             \
  ::Moose::UnitUtils::throwsWithMessage<exc_type, true, true, false>(                              \
      [&]() { stmt; }, expected_msg, __FILE__, __LINE__)

/// Expect that an action throws with a partial message
#define EXPECT_THROW_MSG_CONTAINS(stmt, exc_type, expected_substr)                                 \
  ::Moose::UnitUtils::throwsWithMessage<exc_type, false, false, false>(                            \
      [&]() { stmt; }, expected_substr, __FILE__, __LINE__)

/// Assert that an action throws with a partial message
#define ASSERT_THROW_MSG_CONTAINS(stmt, exc_type, expected_substr)                                 \
  ::Moose::UnitUtils::throwsWithMessage<exc_type, false, true, false>(                             \
      [&]() { stmt; }, expected_substr, __FILE__, __LINE__)

/// Expect that a mooseError is thrown with an exact message
#define EXPECT_MOOSEERROR_MSG(stmt, expected_msg)                                                  \
  ::Moose::UnitUtils::throwsWithMessage<MooseRuntimeError, true, false, true>(                     \
      [&]() { stmt; }, expected_msg, __FILE__, __LINE__)

/// Assert that an mooseError is thrown with an exact message
#define ASSERT_MOOSEERROR_MSG(stmt, expected_msg)                                                  \
  ::Moose::UnitUtils::throwsWithMessage<MooseRuntimeError, true, true, true>(                      \
      [&]() { stmt; }, expected_msg, __FILE__, __LINE__)

/// Expect that an mooseError is thrown with a partial message
#define EXPECT_MOOSEERROR_MSG_CONTAINS(stmt, expected_substr)                                      \
  ::Moose::UnitUtils::throwsWithMessage<MooseRuntimeError, false, false, true>(                    \
      [&]() { stmt; }, expected_substr, __FILE__, __LINE__)

/// Assert that an mooseError is thrown with a partial message
#define ASSERT_MOOSEERROR_MSG_CONTAINS(stmt, expected_substr)                                      \
  ::Moose::UnitUtils::throwsWithMessage<MooseRuntimeError, false, true, true>(                     \
      [&]() { stmt; }, expected_substr, __FILE__, __LINE__)
