//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Capability.h"

#include <map>
#include <set>
#include <string>
#include <utility>

#ifdef MOOSE_UNIT_TEST
class CapabilitiesTest;
#endif

namespace Moose::internal
{

class Capabilities;

/**
 * Registry of capabilities that checks capability requirements.
 *
 * This registry is used both within MOOSE (in framework/src/base/Capabilities.C)
 * and within the python interface (in python/pycapabilities/_pycapabilities.C).
 */
class CapabilityRegistry
{
public:
  /// Capabilities that are reserved and can only be augmented
  static const std::set<std::string, std::less<>> augmented_capability_names;

  ~CapabilityRegistry() = default;

  /// Type for the registry
  using RegistryType = std::map<std::string, Capability, std::less<>>;

  /**
   * Return state for check. We use a plain enum because we rely on implicit conversion to int.
   * Capability checks are run in the test harness using the JSON dump exported from the
   * executable using `--show-capabilities`. This static check does not take dynamic loading into
   * account, as capabilities that would be registered after initializing the dynamically loaded
   * application will not exist with `--show-capabilities`.
   *
   * A requested capability that is not registered at all is considered in a "possible" state,
   * as we cannot guarantee that it does or not exist with a dynamic application. If no dynamic
   * application loading is used, the possible states can be considered certain states.
   *
   * When the test harness Tester specification "dynamic_capabilities" is set to True,
   * it will run the test unless the result of the check is CERTAIN_FAIL. In this case,
   * the runtime check in the executable will terminate if the result is either CERTAIN_FAIL
   * or POSSIBLE_FAIL.
   */
  enum CheckState
  {
    CERTAIN_FAIL = 0,
    POSSIBLE_FAIL = 1,
    UNKNOWN = 2,
    POSSIBLE_PASS = 3,
    CERTAIN_PASS = 4
  };

  /**
   * Storage for the result from check().
   */
  struct CheckResult
  {
    /// State of the check
    CheckState state;
    /// Reason associated with the check (currently unused)
    std::string reason;
    /// Documentation associated with the check (currently unused)
    std::string doc;
  };

  /**
   * Add a capability.
   *
   * @param registry The registry
   * @param capability The name of the capability
   * @param value The value of the capability
   * @param doc The documentation string
   * @return The capability
   */
  Capability & add(const std::string_view name,
                   const Moose::Capability::Value & value,
                   const std::string_view doc);

  /**
   * Query a capability, if it exists, otherwise nullptr.
   *
   * Will convert the capability name to lowercase.
   */
  ///@{
  const Capability * query(std::string capability) const;
#if defined(MOOSE_UNIT_TEST) || defined(FOR_PYCAPABILITIES)
  inline Capability * query(std::string capability);
#endif
  ///@}

  /**
   * Get a capability.
   *
   * Will convert the capability name to lowercase.
   */
  ///@{
  const Capability & get(const std::string & capability) const;
#ifdef MOOSE_UNIT_TEST
  inline Capability & get(const std::string & capability);
#endif
  ///@}

  /**
   * @return The size of the registry (number of capabilities registered).
   */
  std::size_t size() const { return _registry.size(); }

  /**
   * Checks if a set of requirements is satisified by the capabilities
   *
   * @param capabilities The registry that contains the capabilities
   * @param requirements The requirement string
   *
   * This method is exposed to Python within pycapabilities.Capabilities.check in
   * python/pycapabilities/_pycapabilities.C. This external method is used
   * significantly by the TestHarness to check capabilities for individual test specs.
   *
   * Additionally, this method is used by the MooseApp command line option
   * "--required-capabilities ...".
   *
   * Requirements can use comparison operators (>,<,>=,<=,=!,=), where the name of
   * the capability must always be on the left hand side. Comparisons can be performed
   * on strings "compiler!=GCC" (which are case insensitive), integer numbers
   * "ad_size>=50", and version numbers "petsc>3.8.0". The state of a boolean
   * valued capability can be tested by just specifying the capability name "chaco".
   * This check can be inverted using the ! operator as "!chaco".
   *
   * The logic operators & and | can be used to chain multiple checks as
   * "thermochimica & thermochimica>1.0". Parenthesis can be used to build
   * complex logic expressions.
   *
   * See the description for CheckState for more information on why a
   * certain state would be returned.
   */
  CheckResult check(std::string requirements) const;

protected:
#ifdef MOOSE_UNIT_TEST
  friend class ::CapabilitiesTest;
#endif

  /// Registry storage
  RegistryType _registry;
};

#if defined(MOOSE_UNIT_TEST) || defined(FOR_PYCAPABILITIES)
Capability *
CapabilityRegistry::query(std::string capability)
{
  return const_cast<Capability *>(std::as_const(*this).query(capability));
}
#endif

#ifdef MOOSE_UNIT_TEST
Capability &
CapabilityRegistry::get(const std::string & capability)
{
  return const_cast<Capability &>(std::as_const(*this).get(capability));
}
#endif
} // namespace Moose::internal
