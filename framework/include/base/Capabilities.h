//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CapabilityUtils.h"

#ifdef MOOSE_UNIT_TEST
// forward declare unit tests
#include "gtest/gtest.h"
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, boolTest);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, intTest);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, stringTest);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, versionTest);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, multipleTest);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, parseFail);
#endif

namespace Moose
{

/**
 * This singleton class holds a registry for capabilities supported by the current app.
 * A capability can refer to an optional library or optional functionality. Capabilities
 * can either be registered as boolean values (present or not), an integer quantity (like
 * the AD backing store size), a string (like the compiler used to build the executable),
 * or a version number (numbers separated by dots, e.g. the petsc version).
 */
class Capabilities
{
public:
  virtual ~Capabilities() {}

  static Capabilities & getCapabilityRegistry();

  /// register a new capability
  void add(const std::string & capability, CapabilityUtils::Type value, const std::string & doc);
  void add(const std::string & capability, const char * value, const char * doc);

  /// create a JSON dump of the capabilities registry
  std::string dump() const;

  /// check if the given required capabilities are fulfilled, returns a bool, a reason, and a verbose documentation
  CapabilityUtils::Result check(const std::string & requested_capabilities) const;

  ///@{ Don't allow creation through copy/move construction or assignment
  Capabilities(Capabilities const &) = delete;
  Capabilities & operator=(Capabilities const &) = delete;

  Capabilities(Capabilities &&) = delete;
  Capabilities & operator=(Capabilities &&) = delete;
  ///@}

protected:
  /**
   * Capability registry. The capabilities registered here can be dumped using the
   * --show-capabilities command line option. Capabilities are used by the test harness
   * to conditionally enable/disable tests that rely on optional capabilities.
   */
  std::map<std::string, std::pair<CapabilityUtils::Type, std::string>> _capability_registry;

private:
  // Private constructor for singleton pattern
  Capabilities() {}

#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(::CapabilitiesTest, boolTest);
  FRIEND_TEST(::CapabilitiesTest, intTest);
  FRIEND_TEST(::CapabilitiesTest, stringTest);
  FRIEND_TEST(::CapabilitiesTest, versionTest);
  FRIEND_TEST(::CapabilitiesTest, multipleTest);
  FRIEND_TEST(::CapabilitiesTest, parseFail);
#endif
};

} // namespace Moose
