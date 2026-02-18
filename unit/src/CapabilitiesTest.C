//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseUnitUtils.h"

#include "AppFactory.h"
#include "Capabilities.h"
#include "CapabilityException.h"

#include "nlohmann/json.h"

using Moose::Capability;
using Moose::internal::Capabilities;
using Moose::internal::CapabilityRegistry;
using CheckState = Moose::internal::CapabilityRegistry::CheckState;

class CapabilitiesTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Setup a temporary Capabilities for testing
    std::swap(Capabilities::getCapabilities({})._registry, _old_registry);
    _capabilities = &Capabilities::getCapabilities({});
    ASSERT_EQ(Capabilities::getCapabilities({})._registry.size(), 0);
  }

  void TearDown() override
  {
    // Replace temporary Capabilities for testing with real one
    std::swap(Capabilities::getCapabilities({})._registry, _old_registry);
    _old_registry.clear();
    _capabilities = nullptr;
  }

  /// Pointer to the current Capabilities during a test
  Capabilities * _capabilities = nullptr;
  /// Temporary storage for the actual Capabilities during tests
  Capabilities::RegistryType _old_registry;
};

/// Test MooseApp::addBoolCapability
TEST_F(CapabilitiesTest, mooseAppAddBoolCapability)
{
  // success
  const auto capability = &MooseApp::addBoolCapability("name", false, "doc");
  EXPECT_EQ(_capabilities->query("name"), capability);

  // exceptions are moose errors
  EXPECT_MOOSEERROR_MSG(MooseApp::addBoolCapability("name", true, "doc"),
                        "Capability 'name' already exists and is not equal");
}

/// Test MooseApp::addIntCapability
TEST_F(CapabilitiesTest, mooseAppAddIntCapability)
{
  // success
  const auto capability = &MooseApp::addIntCapability("name", 1, "doc");
  EXPECT_EQ(_capabilities->query("name"), capability);

  // catch any exceptions and report as a mooseError
  EXPECT_MOOSEERROR_MSG(MooseApp::addIntCapability("name", 2, "doc"),
                        "Capability 'name' already exists and is not equal");
}

/// Test MooseApp::addStringCapability
TEST_F(CapabilitiesTest, mooseAppAddStringCapability)
{
  // success
  const auto capability = &MooseApp::addStringCapability("name", "value", "doc");
  EXPECT_EQ(_capabilities->query("name"), capability);

  // catch any exceptions and report as a mooseError
  EXPECT_MOOSEERROR_MSG(MooseApp::addStringCapability("name", "foo", "doc"),
                        "Capability 'name' already exists and is not equal");
}

/// Test MooseApp::addCapability
TEST_F(CapabilitiesTest, mooseAppAddCapability)
{
  // can't add, deprecation warning
  {
    Moose::ScopedDeprecatedIsError deprecated_is_error(true);
    EXPECT_MOOSEERROR_MSG_CONTAINS(
        MooseApp::addCapability("name", "foo", "doc"),
        "Deprecated code:\nMooseApp::addCapability() is deprecated (adding capability 'name'); "
        "use one of MooseApp::add[Bool,Int,String]Capability instead.");
  }

  // can add, ignore deprecation warning
  const Capability * capability;
  {
    Moose::ScopedDeprecatedIsError deprecated_is_error(false);
    capability = &MooseApp::addCapability("name", "value", "doc");
    EXPECT_EQ(_capabilities->query("name"), capability);
  }

  // adding the second time gives no deprecation warning
  {
    Moose::ScopedDeprecatedIsError deprecated_is_error(true);
    MooseApp::addCapability("name", "value", "doc");
    EXPECT_EQ(_capabilities->query("name"), capability);
  }
}

/// Test MooseApp::isRelocated
TEST_F(CapabilitiesTest, mooseAppIsRelocated)
{
  // always in-tree with unit tests
  _capabilities->registerMooseCapabilities();
  EXPECT_FALSE(MooseApp::isRelocated());
}

/// Test MooseApp::isInTree
TEST_F(CapabilitiesTest, mooseAppisInTree)
{
  // always in-tree with unit tests
  _capabilities->registerMooseCapabilities();
  EXPECT_TRUE(MooseApp::isInTree());
}

/// Test --check-capabilities in MooseApp
TEST_F(CapabilitiesTest, mooseAppCheckCapabilities)
{
  auto & capabilities = Capabilities::getCapabilities({});
  capabilities.add("value_true", bool(true), "doc");
  capabilities.add("value_false", bool(false), "doc");

  // Are fulfilled
  {
    auto app =
        AppFactory::create("MooseUnitApp", {"--check-capabilities='value_true & !value_false'"});
    app->run();
    EXPECT_EQ(app->exitCode(), 0);
  }

  // Aren't fulfilled
  {
    auto app =
        AppFactory::create("MooseUnitApp", {"--check-capabilities='!value_true | value_false'"});
    app->run();
    EXPECT_EQ(app->exitCode(), 77);
  }

  // Exceptions caught as mooseError
  {
    auto app = AppFactory::create("MooseUnitApp", {"--check-capabilities='foo!?'"});
    EXPECT_MOOSEERROR_MSG_CONTAINS(
        app->run(), "--check-capablities: Capability statement '!': unknown operator.");
  }
}

/// Test --required-capabilities in MooseApp
TEST_F(CapabilitiesTest, mooseAppCheckRequiredCapabilities)
{
  auto & capabilities = Capabilities::getCapabilities({});
  capabilities.add("value_true", bool(true), "doc");
  capabilities.add("value_false", bool(false), "doc");

  // Aren't fulfilled
  {
    auto app =
        AppFactory::create("MooseUnitApp", {"--required-capabilities='!value_true | value_false'"});
    app->run();
    EXPECT_EQ(app->exitCode(), 77);
  }

  // Exceptions caught as mooseError
  {
    auto app = AppFactory::create("MooseUnitApp", {"--required-capabilities='foo!?'"});
    EXPECT_MOOSEERROR_MSG_CONTAINS(
        app->run(), "--required-capablities: Capability statement '!': unknown operator.");
  }

  // Unknown state
  {
    auto app = AppFactory::create("MooseUnitApp", {"--required-capabilities='foo>1'"});
    EXPECT_MOOSEERROR_MSG_CONTAINS(app->run(), "are not specific enough");
  }
}

// Set of dumped capabilities that stores a reasonable
// range of all Capability types
const nlohmann::json JSON_CAPABILITIES = {
    {"false", {{"doc", "false"}, {"value", false}}},
    {"int", {{"doc", "1"}, {"explicit", false}, {"value", 1}}},
    {"int_explicit", {{"doc", "1"}, {"explicit", true}, {"value", 1}}},
    {"string", {{"doc", "string"}, {"explicit", false}, {"value", "string"}}},
    {"string_enum",
     {{"doc", "string_enum"},
      {"enumeration", {"foo", "string_enum"}},
      {"explicit", false},
      {"value", "string_enum"}}},
    {"string_explicit",
     {{"doc", "string_explicit"}, {"explicit", true}, {"value", "string_explicit"}}},
    {"string_explicit_enum",
     {{"doc", "string_explicit_enum"},
      {"enumeration", {"foo", "string_explicit_enum"}},
      {"explicit", true},
      {"value", "string_explicit_enum"}}},
    {"true", {{"doc", "true"}, {"value", true}}}};

/// Test --testharness-capabilities in MooseApp
TEST_F(CapabilitiesTest, mooseAppTestharnessCapabilities)
{
  // Check string that satisfies each entry in JSON_CAPABILITIES
  const std::string check_capabilities =
      "--check-capabilities='!false & int & int=1 & int_explicit=1 & string & string=string & "
      "string_enum & string_enum=string_enum & string_explicit=string_explicit & "
      "string_explicit_enum=string_explicit_enum & true'";

  // Check fails without augmenting via --testharness-capabilities
  {
    auto app = AppFactory::create("MooseUnitApp", {check_capabilities});
    app->run();
    EXPECT_EQ(app->exitCode(), 77);
  }

  // Success once adding --testharness-capabilities to augment
  {
    Moose::UnitUtils::TempFile temp_file;

    std::ofstream out(temp_file.path());
    out << JSON_CAPABILITIES.dump() << std::flush;
    out.close();

    auto app = AppFactory::create(
        "MooseUnitApp", {"--testharness-capabilities", temp_file.path(), check_capabilities});
    app->run();
    EXPECT_EQ(app->exitCode(), 0);
  }

  // File doesn't exist
  {
    const std::string bad_file = "/testharness/capabilities/no/exist.json";
    auto app = AppFactory::create("MooseUnitApp", {"--testharness-capabilities", bad_file});
    EXPECT_MOOSEERROR_MSG_CONTAINS(
        app->run(), "--testharness-capabilities: Could not open \"" + bad_file + "\"");
  }

  // Bad JSON parse, caught as mooseError
  {
    Moose::UnitUtils::TempFile temp_file;

    std::ofstream out(temp_file.path());
    out << "{" << std::flush;
    out.close();

    auto app = AppFactory::create("MooseUnitApp", {"--testharness-capabilities", temp_file.path()});
    EXPECT_MOOSEERROR_MSG_CONTAINS(app->run(),
                                   "--testharness-capabilities: Failed to load capabilities \"" +
                                       std::string(temp_file.path()) +
                                       "\":\n[json.exception.parse_error");
  }

  // Bad augment, caught as mooseError
  {
    Moose::UnitUtils::TempFile temp_file;

    const nlohmann::json root = {{"name", {{"foo", "bar"}}}};
    std::ofstream out(temp_file.path());
    out << root.dump() << std::flush;
    out.close();

    auto app = AppFactory::create("MooseUnitApp", {"--testharness-capabilities", temp_file.path()});
    EXPECT_MOOSEERROR_MSG_CONTAINS(
        app->run(),
        "--testharness-capabilities: Failed to load capabilities \"" +
            std::string(temp_file.path()) +
            "\":\nCapabilities::augment: Capability 'name' missing 'doc' entry");
  }
}

/// Test Capabilities::dump
TEST_F(CapabilitiesTest, dump)
{
  auto & capabilities = Capabilities::getCapabilities({});

  capabilities.add("false", bool(false), "false");
  capabilities.add("true", bool(true), "true");
  capabilities.add("int", int(1), "1");
  capabilities.add("int_explicit", int(1), "1").setExplicit();
  capabilities.add("string", std::string("string"), "string");
  capabilities.add("string_explicit", std::string("string_explicit"), "string_explicit")
      .setExplicit();
  capabilities.add("string_enum", std::string("string_enum"), "string_enum")
      .setEnumeration({"string_enum", "foo"});
  capabilities
      .add("string_explicit_enum", std::string("string_explicit_enum"), "string_explicit_enum")
      .setExplicit()
      .setEnumeration({"string_explicit_enum", "foo"});

  const auto dumped = capabilities.dump();
  const auto loaded = nlohmann::json::parse(dumped);

  EXPECT_EQ(loaded, JSON_CAPABILITIES);
}

/// Test Capabilities::check
TEST_F(CapabilitiesTest, check)
{
  auto & capabilities = Capabilities::getCapabilities({});

  capabilities.add("name", bool(true), "false");
  EXPECT_EQ(capabilities.check("name").state, CheckState::CERTAIN_PASS);
  EXPECT_EQ(capabilities.check("!name").state, CheckState::CERTAIN_FAIL);
}

/// Test Capabilities::augment
TEST_F(CapabilitiesTest, augment)
{
  auto & capabilities = Capabilities::getCapabilities({});

  capabilities.augment(JSON_CAPABILITIES, {});

  const auto test_capability = [this](const std::string & name,
                                      const Capability::Value & value,
                                      const std::string & doc,
                                      const bool is_explicit = false,
                                      const std::optional<std::set<std::string>> & enumeration = {})
  {
    const auto capability_ptr = _capabilities->query(name);
    EXPECT_NE(capability_ptr, nullptr);
    const auto & capability = *capability_ptr;
    EXPECT_EQ(capability.getName(), name);
    EXPECT_EQ(capability.getValue(), value);
    EXPECT_EQ(capability.getDoc(), doc);
    EXPECT_EQ(capability.getExplicit(), is_explicit);
    if (enumeration)
    {
      auto & cap_enumeration_ptr = capability.queryEnumeration();
      EXPECT_TRUE(cap_enumeration_ptr.has_value());
      EXPECT_EQ(*cap_enumeration_ptr, *enumeration);
    }
  };

  test_capability("false", bool(false), "false");
  test_capability("int", int(1), "1");
  test_capability("int_explicit", int(1), "1", true);
  test_capability("string", std::string("string"), "string");
  test_capability("string_enum",
                  std::string("string_enum"),
                  "string_enum",
                  false,
                  std::set<std::string>{"string_enum", "foo"});
  test_capability("string_explicit", std::string("string_explicit"), "string_explicit", true);
  test_capability("string_explicit_enum",
                  std::string("string_explicit_enum"),
                  "string_explicit_enum",
                  true,
                  std::set<std::string>{"string_explicit_enum", "foo"});
  test_capability("true", bool(true), "true");
}

/// Test Capabilities::augment with a parsing failure
TEST_F(CapabilitiesTest, augmentParseError)
{
  auto & capabilities = Capabilities::getCapabilities({});

  // missing doc
  {
    const nlohmann::json root = {{"name", {{"value", false}}}};
    EXPECT_THROW_MSG(capabilities.augment(root, {}),
                     Moose::CapabilityException,
                     "Capabilities::augment: Capability 'name' missing 'doc' entry");
  }

  // missing value
  {
    const nlohmann::json root = {{"name", {{"doc", "foo"}}}};
    EXPECT_THROW_MSG(capabilities.augment(root, {}),
                     Moose::CapabilityException,
                     "Capabilities::augment: Capability 'name' missing 'value' entry");
  }
}

/// Test Capabilities::isRelocated with errors
TEST_F(CapabilitiesTest, isInstallationTypeErrors)
{
  auto & capabilities = Capabilities::getCapabilities({});

  // capability does not exist
  EXPECT_THROW_MSG(
      capabilities.isInstallationType("unused"),
      Moose::CapabilityException,
      "Capabilities::isInstallationType(): Capability 'installation_type' is not registered");

  // capability is not a string
  capabilities.add("installation_type", bool(false), "foo");
  EXPECT_THROW_MSG(
      capabilities.isInstallationType("unused"),
      Moose::CapabilityException,
      "Capabilities::isInstallationType(): Capability 'installation_type' is not a string");
}

/// Test Capabilities::[isInstallationType,isRelocated,isInTree]
TEST_F(CapabilitiesTest, isInstallationType)
{
  auto & capabilities = Capabilities::getCapabilities({});

  // We test unit tests in tree, so doing the normal registration
  // with moose should get us installation_type=in_tree
  capabilities.registerMooseCapabilities();
  Capability & capability = capabilities.get("installation_type");

  // Should be false with installation_type=in_tree
  EXPECT_FALSE(capabilities.isInstallationType("foo"));
  EXPECT_FALSE(capabilities.isInstallationType("relocated"));
  EXPECT_TRUE(capabilities.isInstallationType("in_tree"));
  EXPECT_TRUE(capabilities.isInTree());
  EXPECT_FALSE(capabilities.isRelocated());

  // Set the value to "relocated"
  const auto enumeration_ptr = capability.queryEnumeration();
  ASSERT_TRUE(enumeration_ptr.has_value());
  const auto enumeration = *enumeration_ptr;
  capability = Capability(capability.getName(), std::string("relocated"), capability.getDoc())
                   .setExplicit()
                   .setEnumeration(enumeration);
  ASSERT_EQ(capability.getStringValue(), "relocated");
  EXPECT_FALSE(capabilities.isInstallationType("foo"));
  EXPECT_TRUE(capabilities.isInstallationType("relocated"));
  EXPECT_FALSE(capabilities.isInstallationType("in_tree"));
  EXPECT_FALSE(capabilities.isInTree());
  EXPECT_TRUE(capabilities.isRelocated());
}
