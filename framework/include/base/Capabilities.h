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
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, check);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, dump);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, isInstallationTypeErrors);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, isInstallationType);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, mooseAppCheckCapabilities);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, mooseAppCheckRequiredCapabilities);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, mooseAppIsRelocated);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, mooseAppisInTree);
class GTEST_TEST_CLASS_NAME_(RegistryTest, addDataFilePath);
class GTEST_TEST_CLASS_NAME_(RegistryTest, addMissingDataFilePath);
class CapabilitiesTest;
class DataFileUtilsTest;
class RegistryTest;
#endif

class AppFactory;
class Registry;
class MooseApp;

namespace Moose::internal
{

/**
 * Holds the public (to MooseApp) facing CapabilityRegistry for storing and checking capabilities.
 *
 * A capability can refer to an optional library or optional functionality. Capabilities
 * can either be registered as boolean values (present or not), an integer quantity (like
 * the AD backing store size), a string (like the compiler used to build the executable),
 * or a version number (numbers separated by dots, e.g. the petsc version).
 */
class Capabilities : public CapabilityRegistry
{
public:
  /// Passkey for get()
  class GetCapabilitiesPassKey
  {
    friend AppFactory;
    friend MooseApp;
    friend Registry;
#ifdef MOOSE_UNIT_TEST
    friend class ::CapabilitiesTest;
    friend class ::DataFileUtilsTest;
    friend class ::RegistryTest;
    FRIEND_TEST(::CapabilitiesTest, augment);
    FRIEND_TEST(::CapabilitiesTest, augmentParseError);
    FRIEND_TEST(::CapabilitiesTest, check);
    FRIEND_TEST(::CapabilitiesTest, dump);
    FRIEND_TEST(::CapabilitiesTest, isInstallationTypeErrors);
    FRIEND_TEST(::CapabilitiesTest, isInstallationType);
    FRIEND_TEST(::CapabilitiesTest, mooseAppCheckCapabilities);
    FRIEND_TEST(::CapabilitiesTest, mooseAppCheckRequiredCapabilities);
    FRIEND_TEST(::RegistryTest, addDataFilePath);
    FRIEND_TEST(::RegistryTest, addMissingDataFilePath);
#endif
    GetCapabilitiesPassKey() {}
    GetCapabilitiesPassKey(const GetCapabilitiesPassKey &) {}
  };

  /**
   * Get the singleton Capabilities.
   *
   * Only accessible through MooseApp and AppFactory. Addition
   * of capabilities should be done through the
   * MooseApp::add[Bool,Int,String]capability() method.
   */
  static Capabilities & getCapabilities(const GetCapabilitiesPassKey);

  /// create a JSON dump of the capabilities registry
  std::string dump() const;

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

  /**
   * @return Whether or not the application is relocated
   */
  bool isRelocated() const { return isInstallationType("relocated"); }

  /**
   * @return Whether or not the application is in-tree
   */
  bool isInTree() const { return isInstallationType("in_tree"); }

private:
#ifdef MOOSE_UNIT_TEST
  friend class ::CapabilitiesTest;
  friend class ::DataFileUtilsTest;
  friend class ::RegistryTest;
  FRIEND_TEST(::CapabilitiesTest, isInstallationTypeErrors);
  FRIEND_TEST(::CapabilitiesTest, isInstallationType);
  FRIEND_TEST(::CapabilitiesTest, mooseAppIsRelocated);
  FRIEND_TEST(::CapabilitiesTest, mooseAppisInTree);
#endif

  /**
   * Helper for isRelocated() and isInTree()
   */
  bool isInstallationType(const std::string & installation_type) const;

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

  // Private constructor for singleton pattern
  Capabilities();
};

} // namespace Moose::internal
