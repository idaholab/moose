//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CapabilityRegistry.h"

#include "nlohmann/json_fwd.h"

#ifdef MOOSE_UNIT_TEST
// forward declare unit tests
#include "gtest/gtest.h"
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, augment);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, augmentParseError);
class CapabilitiesTest;
#endif

class MooseApp;

namespace Moose::internal
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
  /**
   * Get the singleton Capabilities.
   */
  static Capabilities & get();

  /**
   * Add a capability to the registry.
   *
   * @param capability The name of the capability
   * @param value The value of the capability
   * @param doc The documentation string
   * @return The capability
   */
  Moose::Capability & add(const std::string_view capability,
                          const Moose::Capability::Value & value,
                          const std::string_view doc);

  /**
   * Query a capability, if it exists, otherwise nullptr.
   *
   * @param capability The name of the capability.
   */
  const Moose::Capability * query(const std::string & capability) const;

  /**
   * Get a capability.
   *
   * @param capability The name of the capability
   */
  const Moose::Capability & get(const std::string & capability) const;

  /// create a JSON dump of the capabilities registry
  std::string dump() const;

  /// check if the given required capabilities are fulfilled, returns a bool, a reason, and a verbose documentation
  Moose::internal::CapabilityRegistry::CheckResult
  check(const std::string & requested_capabilities) const;

  /// Passkey for augment()
  class AugmentPassKey
  {
    friend MooseApp;
#ifdef MOOSE_UNIT_TEST
    FRIEND_TEST(::CapabilitiesTest, augment);
    FRIEND_TEST(::CapabilitiesTest, augmentParseError);
#endif
    AugmentPassKey() {}
    AugmentPassKey(const AugmentPassKey &) {}
  };

  /**
   * Augment the capabilities with the given input capabilities.
   *
   * This is used when loading additional capabilities at run time
   * from the TestHarness and thus is only allowed to be used by
   * the MooseApp.
   */
  void augment(const nlohmann::json & input, const AugmentPassKey);

  ///@{ Don't allow creation through copy/move construction or assignment
  Capabilities(Capabilities const &) = delete;
  Capabilities & operator=(Capabilities const &) = delete;

  Capabilities(Capabilities &&) = delete;
  Capabilities & operator=(Capabilities &&) = delete;
  ///@}

private:
#ifdef MOOSE_UNIT_TEST
  friend class ::CapabilitiesTest;
#endif

  /**
   * Register the MOOSE capabilities.
   *
   * Called during the construction of the registry.
   *
   * Putting this here enforces that only capabilities that
   * represent context _before_ the app is constructed
   * can be added.
   */
  void registerMooseCapabilities();

  /**
   * Capability registry. The capabilities registered here can be dumped using the
   * --show-capabilities command line option. Capabilities are used by the test harness
   * to conditionally enable/disable tests that rely on optional capabilities.
   */
  Moose::internal::CapabilityRegistry _capability_registry;

  // Private constructor for singleton pattern
  Capabilities();
};

} // namespace Moose::internal
