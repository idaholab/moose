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

#include "MooseApp.h"
#include "Capabilities.h"

#include "nlohmann/json.h"

using Moose::CapabilityUtils::Capability;
using Moose::CapabilityUtils::CapabilityException;
using Moose::CapabilityUtils::CapabilityRegistry;
using Moose::CapabilityUtils::CapabilityValue;
using Moose::CapabilityUtils::CERTAIN_FAIL;
using Moose::CapabilityUtils::CERTAIN_PASS;
using Moose::CapabilityUtils::CheckState;
using Moose::CapabilityUtils::POSSIBLE_FAIL;
using Moose::CapabilityUtils::POSSIBLE_PASS;
using Moose::CapabilityUtils::UNKNOWN;

#define CAP_EXPECT_THROW_MSG(statement, message)                                                   \
  EXPECT_THROW_MSG(statement, CapabilityException, message);

/// Test Moose::CapabilityUtils::Capability() and getter state
TEST(CapabilityTest, capabilityConstructor)
{
  // empty name
  CAP_EXPECT_THROW_MSG(Capability("", true, "doc"), "Capability has empty name");

  // disallowed characters
  CAP_EXPECT_THROW_MSG(
      Capability("A!", true, "doc"),
      "Capability 'A!': Name has unallowed characters; allowed characters = 'a-z, 0-9, _, -'");

  // string value has disallowed characters
  CAP_EXPECT_THROW_MSG(Capability("cap", "A!", "doc"),
                       "String capability 'cap': value 'A!' has unallowed characters; allowed "
                       "characters = 'a-z, 0-9, _, ., -'");

  // bool false value
  {
    Capability cap("name", false, "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
    EXPECT_EQ(cap.getDoc(), "doc");
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_NE(cap.queryBoolValue(), nullptr);
    EXPECT_EQ(*cap.queryBoolValue(), false);
    EXPECT_EQ(cap.queryIntValue(), nullptr);
    EXPECT_EQ(cap.queryStringValue(), nullptr);
    EXPECT_TRUE(cap.hasBoolValue());
    EXPECT_FALSE(cap.hasIntValue());
    EXPECT_FALSE(cap.hasStringValue());
    EXPECT_EQ(cap.valueToString(), "false");
    EXPECT_EQ(cap.toString(), "name=false");
  }

  // bool true value
  {
    Capability cap("name", true, "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), CapabilityValue{true});
    EXPECT_EQ(cap.getDoc(), "doc");
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_NE(cap.queryBoolValue(), nullptr);
    EXPECT_EQ(*cap.queryBoolValue(), true);
    EXPECT_EQ(cap.queryIntValue(), nullptr);
    EXPECT_EQ(cap.queryStringValue(), nullptr);
    EXPECT_TRUE(cap.hasBoolValue());
    EXPECT_FALSE(cap.hasIntValue());
    EXPECT_FALSE(cap.hasStringValue());
    EXPECT_EQ(cap.valueToString(), "true");
    EXPECT_EQ(cap.toString(), "name=true");
  }

  // int value
  {
    Capability cap("name", 1, "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), CapabilityValue{1});
    EXPECT_EQ(cap.getDoc(), "doc");
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_EQ(cap.queryBoolValue(), nullptr);
    EXPECT_NE(cap.queryIntValue(), nullptr);
    EXPECT_EQ(*cap.queryIntValue(), 1);
    EXPECT_EQ(cap.queryStringValue(), nullptr);
    EXPECT_FALSE(cap.hasBoolValue());
    EXPECT_TRUE(cap.hasIntValue());
    EXPECT_FALSE(cap.hasStringValue());
    EXPECT_EQ(cap.valueToString(), "1");
    EXPECT_EQ(cap.toString(), "name=1");
  }

  // string value
  {
    Capability cap("name", "foo", "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), CapabilityValue{"foo"});
    EXPECT_EQ(cap.getDoc(), "doc");
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_EQ(cap.queryBoolValue(), nullptr);
    EXPECT_EQ(cap.queryIntValue(), nullptr);
    EXPECT_NE(cap.queryStringValue(), nullptr);
    EXPECT_EQ(*cap.queryStringValue(), "foo");
    EXPECT_FALSE(cap.hasBoolValue());
    EXPECT_FALSE(cap.hasIntValue());
    EXPECT_TRUE(cap.hasStringValue());
    EXPECT_EQ(cap.valueToString(), "foo");
    EXPECT_EQ(cap.toString(), "name=foo");
  }
}

/// Test Moose::CapabilityUtils::Capability enumeration state
TEST(CapabilityTest, capabilityEnumeration)
{
  // can't set an enumeration for a bool
  CAP_EXPECT_THROW_MSG(Capability("name", false, "doc").setEnumeration({}),
                       "Capability::setEnumeration(): Capability 'name' is not string-valued and "
                       "cannot have an enumeration");

  // can't set an enumeration for an int
  CAP_EXPECT_THROW_MSG(Capability("name", 1, "doc").setEnumeration({}),
                       "Capability::setEnumeration(): Capability 'name' is not string-valued and "
                       "cannot have an enumeration");

  // check set
  {
    const std::vector<std::string> enumeration{"foo", "bar"};
    Capability cap("name", "foo", "doc");

    // initial set
    cap.setEnumeration(enumeration);
    EXPECT_EQ(cap.getEnumeration(), enumeration);
    EXPECT_TRUE(cap.hasEnumeration("foo"));
    EXPECT_TRUE(cap.hasEnumeration("bar"));
    EXPECT_FALSE(cap.hasEnumeration("baz"));

    // enumeration to string
    EXPECT_EQ(cap.enumerationToString(), "foo, bar");

    // setting again ignores checks
    cap.setEnumeration(enumeration);
    EXPECT_EQ(cap.getEnumeration(), enumeration);

    // setting again to a different enumeration not allowed
    CAP_EXPECT_THROW_MSG(
        cap.setEnumeration({"baz"}),
        "Capability::setEnumeration(): Capability 'name' already has an enumeration set");
  }

  // cannot be empty
  CAP_EXPECT_THROW_MSG(Capability("name", "foo", "doc").setEnumeration({}),
                       "Capability::setEnumeration(): Enumeration is empty for 'name'");

  // unallowed characters
  CAP_EXPECT_THROW_MSG(
      Capability("name", "foo", "doc").setEnumeration({"abc!"}),
      "Capability::setEnumeration(): Enumeration value 'abc!' for capability 'name' "
      "has unallowed characters; allowed characters = 'a-z, 0-9, _, -'");
  CAP_EXPECT_THROW_MSG(
      Capability("name", "foo", "doc").setEnumeration({"abc", "def!"}),
      "Capability::setEnumeration(): Enumeration value 'def!' for capability 'name' "
      "has unallowed characters; allowed characters = 'a-z, 0-9, _, -'");

  // duplicates
  CAP_EXPECT_THROW_MSG(
      Capability("name", "foo", "doc").setEnumeration({"foo", "foo"}),
      "Capability::setEnumeration(): Duplicate enumeration 'foo' for capability 'name'");

  // value not in enumeration
  CAP_EXPECT_THROW_MSG(
      Capability("name", "foo", "doc").setEnumeration({"bar"}),
      "Capability::setEnumeration(): Capability name=foo value not within enumeration");

  // getting enumeration for bool capability
  CAP_EXPECT_THROW_MSG(Capability("name", false, "doc").getEnumeration(),
                       "Capability::getEnumeration(): Capability 'name' is not string-valued and "
                       "cannot have an enumeration");
  // getting enumeration for an int capability
  CAP_EXPECT_THROW_MSG(Capability("name", 1, "doc").getEnumeration(),
                       "Capability::getEnumeration(): Capability 'name' is not string-valued and "
                       "cannot have an enumeration");
}

/// Test Moose::CapabilityUtils::Capability explicit state
TEST(CapabilityTest, capabilityExplicit)
{
  // can't set a bool to be explicit
  CAP_EXPECT_THROW_MSG(
      Capability("name", false, "doc").setExplicit(),
      "Capability::setExplicit(): Capability 'name' is bool-valued and cannot be set as explicit");

  // int explicit capability
  EXPECT_TRUE(Capability("name", 1, "doc").setExplicit().getExplicit());

  // string explicit capability
  EXPECT_TRUE(Capability("name", "foo", "doc").setExplicit().getExplicit());
}

/// Test Moose::CapabilityUtils::Capability::negateValue()
TEST(CapabilityTest, capabilityNegateValue)
{
  // negate bool value that is already false
  {
    Capability cap("name", false, "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // negate bool value that is not false
  {
    Capability cap("name", true, "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // negate int value
  {
    Capability cap("name", 1, "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // negate int value that is explicit
  {
    Capability cap("name", 1, "doc");
    cap.setExplicit();
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // string value
  {
    Capability cap("name", "foo", "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // string value that is explicit
  {
    Capability cap("name", "foo", "doc");
    cap.setExplicit();
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // string value that has an enumeration
  {
    Capability cap("name", "foo", "doc");
    cap.setEnumeration({"foo"});
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // string value that has an enumeration and is explicit
  {
    Capability cap("name", "foo", "doc");
    cap.setExplicit();
    cap.setEnumeration({"foo"});
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }
}

#define CAP_CHECK_EXPECT_EQ(requirement, state)                                                    \
  EXPECT_EQ(std::get<0>(registry.check(requirement)), state)

#define CAP_CHECK_EXPECT_ERROR(requirement, error)                                                 \
  CAP_EXPECT_THROW_MSG(registry.check(requirement), error)

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for bool capabilities
TEST(CapabilityRegistryTest, checkBool)
{
  CapabilityRegistry registry;
  registry.add("bool", true, "Boolean test capability");
  registry.add("bool2", false, "Boolean test capability 2");

  const std::vector<std::pair<std::string, CheckState>> tests = {
      {"bool", CERTAIN_PASS},
      {"!bool", CERTAIN_FAIL},
      {"!bool2", CERTAIN_PASS},
      {"bool2", CERTAIN_FAIL},
      {"noexist", POSSIBLE_FAIL},
      {"!noexist", POSSIBLE_PASS},
  };

  for (const auto & [requirement, state] : tests)
    CAP_CHECK_EXPECT_EQ(requirement, state);

  CAP_CHECK_EXPECT_ERROR("bool2=>1.0.0", "Capability statement '=>': unknown operator.");
  CAP_CHECK_EXPECT_ERROR("bool=", "Unable to parse requested capabilities 'bool='.");
  CAP_CHECK_EXPECT_ERROR("bool==", "Unable to parse requested capabilities 'bool=='.");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::checkfor int capabilities
TEST(CapabilityRegistryTest, checkInt)
{
  CapabilityRegistry registry;
  registry.add("int", 78, "Integer test capability");

  const std::vector<std::string> is_true = {
      "", "=78", "==78", "<=78", ">=78", "<=79", ">=77", "<79", ">77", "!=77", "!=79"};
  const std::vector<std::string> is_false = {"!=78", ">=79", "<=77", "<78", ">78", "==77"};

  for (const auto & c : is_true)
  {
    CAP_CHECK_EXPECT_EQ("int" + c, CERTAIN_PASS);
    CAP_CHECK_EXPECT_EQ("!(int" + c + ")", CERTAIN_FAIL);
  }
  for (const auto & c : is_false)
  {
    CAP_CHECK_EXPECT_EQ("int" + c, CERTAIN_FAIL);
    CAP_CHECK_EXPECT_EQ("!(int" + c + ")", CERTAIN_PASS);
  }

  CAP_CHECK_EXPECT_ERROR("int<", "Unable to parse requested capabilities 'int<'.");
  CAP_CHECK_EXPECT_ERROR("int<bla",
                         "Capability statement 'int<bla': capability 'int=78' cannot be "
                         "compared to a string.");
  CAP_CHECK_EXPECT_ERROR("int>1.0",
                         "Capability statement 'int>1.0': capability 'int=78' cannot be "
                         "compared to a version.");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for explicit int capabilities
TEST(CapabilityRegistryTest, checkIntExplicit)
{
  CapabilityRegistry registry;
  registry.add("int", 78, "Integer test capability").setExplicit();

  // should still allow as explicit
  CAP_CHECK_EXPECT_EQ("int=78", CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("int=79", CERTAIN_FAIL);

  // not explicit
  CAP_CHECK_EXPECT_ERROR("int",
                         "Capability statement 'int': capability 'int' requires a value and "
                         "cannot be used in a boolean expression");
  CAP_CHECK_EXPECT_ERROR("!int",
                         "Capability statement 'int': capability 'int' requires a value and "
                         "cannot be used in a boolean expression");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for string capabilities
TEST(CapabilityRegistryTest, checkString)
{
  CapabilityRegistry registry;
  registry.add("string", "clang", "String test capability");

  CAP_CHECK_EXPECT_EQ("string", CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("!string", CERTAIN_FAIL);

  CAP_CHECK_EXPECT_EQ("string=clang", CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("string=CLANG", CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("string=gcc", CERTAIN_FAIL);

  CAP_CHECK_EXPECT_ERROR("string>", "Unable to parse requested capabilities 'string>'.");
  CAP_CHECK_EXPECT_ERROR("string>0",
                         "Capability statement 'string>0': capability 'string=clang' cannot be "
                         "compared to a version.");
  CAP_CHECK_EXPECT_ERROR("string>1.0",
                         "Capability statement 'string>1.0': capability 'string=clang' cannot be "
                         "compared to a version.");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for enumerated string capabilities
TEST(CapabilityRegistryTest, checkEnumeratedString)
{
  CapabilityRegistry registry;
  registry.add("string", "clang", "String test capability").setEnumeration({"clang", "gcc"});

  // should still allow as a bool
  CAP_CHECK_EXPECT_EQ("string", CERTAIN_PASS);

  // within the enumeration
  CAP_CHECK_EXPECT_EQ("string=clang", CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("string=gcc", CERTAIN_FAIL);

  // not within the enumeration
  CAP_CHECK_EXPECT_ERROR("string=foo",
                         "Capability statement 'string=foo': 'foo' invalid for capability "
                         "'string'; valid values: clang, gcc");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for explicit string capabilities
TEST(CapabilityRegistryTest, checkExplicitString)
{
  CapabilityRegistry registry;
  registry.add("string", "clang", "String test capability").setExplicit();

  // should still allow as explicit
  CAP_CHECK_EXPECT_EQ("string=clang", CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("string=foo", CERTAIN_FAIL);

  // not explicit
  CAP_CHECK_EXPECT_ERROR("string",
                         "Capability statement 'string': capability 'string' requires a value and "
                         "cannot be used in a boolean expression");
  CAP_CHECK_EXPECT_ERROR("!string",
                         "Capability statement 'string': capability 'string' requires a value and "
                         "cannot be used in a boolean expression");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for explicit and enumerated string capabilities
TEST(CapabilityRegistryTest, checkExplicitEnumeratedString)
{
  CapabilityRegistry registry;
  registry.add("string", "clang", "String test capability")
      .setEnumeration({"clang", "gcc"})
      .setExplicit();

  // should still allow as explicit
  CAP_CHECK_EXPECT_EQ("string=clang", CERTAIN_PASS);

  // not in enumeration
  CAP_CHECK_EXPECT_ERROR("string=foo",
                         "Capability statement 'string=foo': 'foo' invalid for capability "
                         "'string'; valid values: clang, gcc");

  // not explicit with valid value listing
  CAP_CHECK_EXPECT_ERROR("string",
                         "Capability statement 'string': capability 'string' requires a value and "
                         "cannot be used in a boolean expression; valid values: clang, gcc");
  CAP_CHECK_EXPECT_ERROR("!string",
                         "Capability statement 'string': capability 'string' requires a value and "
                         "cannot be used in a boolean expression; valid values: clang, gcc");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for versioned string capabilities
TEST(CapabilityRegistryTest, checkVersion)
{
  CapabilityRegistry registry;
  registry.add("version", "3.2.1", "Version number test capability");

  const std::vector<std::string> is_true = {"",
                                            ">2.1",
                                            ">3.1",
                                            ">=3.2",
                                            ">3.2.0",
                                            ">=3.2.1",
                                            "<=3.2.1",
                                            "=3.2.1",
                                            "==3.2.1",
                                            "<=3.2.2",
                                            "<3.2.2",
                                            "<3.3.2",
                                            "<4.2.2",
                                            "<3.3",
                                            "<4.2",
                                            "<4"};
  const std::vector<std::string> is_false = {
      "<3", "<2", "<=3", "=1.2.3", "=3", "==4", "==3.2", ">4", ">=4", ">=3.3"};

  for (const auto & c : is_true)
  {
    CAP_CHECK_EXPECT_EQ("version" + c, CERTAIN_PASS);
    CAP_CHECK_EXPECT_EQ("!(version" + c + ")", CERTAIN_FAIL);
  }
  for (const auto & c : is_false)
  {
    CAP_CHECK_EXPECT_EQ("version" + c, CERTAIN_FAIL);
    CAP_CHECK_EXPECT_EQ("!(version" + c + ")", CERTAIN_PASS);
  }
  CAP_CHECK_EXPECT_ERROR("!version<", "Unable to parse requested capabilities '!version<'.");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for multiple valued requirements
TEST(CapabilityRegistryTest, checkMultiple)
{
  CapabilityRegistry registry;
  registry.add("bool", true, "Multiple capability test bool");
  registry.add("int", 78, "Multiple capability test int");
  registry.add("int_explicit", 79, "Multiple capability test int explicit");
  registry.add("string", "clang", "Multiple capability test string");
  registry.add("string_explicit", "foo", "Multiple capability test string explicit").setExplicit();
  registry.add("string_enum", "clang", "Multiple capability test string enum")
      .setEnumeration({"clang", "gcc"});
  registry.add("version", "3.2.1", "Multiple capability test version number");

  CAP_CHECK_EXPECT_EQ("!doesnotexist & version<4.2.2 &"
                      "int<100 & int>50 & string!=Popel ",
                      POSSIBLE_PASS);
  CAP_CHECK_EXPECT_EQ("!doesnotexist & version<4.2.2 &"
                      "int<100 & unittest2_int>50 & string!=Popel ",
                      UNKNOWN);
  CAP_CHECK_EXPECT_EQ("doesnotexist & doesnotexist>2.0.1", POSSIBLE_FAIL);
  CAP_CHECK_EXPECT_EQ("!doesnotexist | doesnotexist<=2.0.1", POSSIBLE_PASS);
  CAP_CHECK_EXPECT_EQ("bool & int!=78", CERTAIN_FAIL);
  CAP_CHECK_EXPECT_EQ("bool | int!=78", CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ(" !bool | (string=gcc | version<1.0)", CERTAIN_FAIL);
  CAP_CHECK_EXPECT_EQ("(bool & int=78) & (string=clang & version>2.0)", CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("bool & string_explicit=foo & int_explicit=78", CERTAIN_FAIL);
  CAP_CHECK_EXPECT_EQ("(string_enum=gcc | string_enum=clang) & bool", CERTAIN_PASS);

  CAP_CHECK_EXPECT_ERROR("bool int< string==Popel",
                         "Unable to parse requested capabilities 'bool int< string==Popel'.");
  CAP_CHECK_EXPECT_ERROR("(bool", "Unable to parse requested capabilities '(bool'.");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for a parse failure
TEST(CapabilityRegistryTest, checkParseFail)
{
  CapabilityRegistry registry;
  CAP_CHECK_EXPECT_ERROR("foo bar", "Unable to parse requested capabilities 'foo bar'.");
}

#undef CAP_CHECK_EXPECT_EQ
#undef CAP_CHECK_EXPECT_ERROR

/// Test Moose::CapabilityUtils::CapabilityRegistry::query
TEST(CapabilityRegistryTest, query)
{
  CapabilityRegistry registry;
  const auto & capability = registry.add("name", false, "doc");

  EXPECT_EQ(registry.query("name"), &capability);
  EXPECT_EQ(registry.query("naMe"), &capability);
  EXPECT_EQ(std::as_const(registry).query("name"), &capability);
  EXPECT_EQ(std::as_const(registry).query("naMe"), &capability);
  EXPECT_EQ(registry.query("foo"), nullptr);
  EXPECT_EQ(std::as_const(registry).query("foo"), nullptr);
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::get
TEST(CapabilityRegistryTest, get)
{
  CapabilityRegistry registry;
  const auto & capability = registry.add("name", false, "doc");

  EXPECT_EQ(&registry.get("name"), &capability);
  CAP_EXPECT_THROW_MSG(registry.get("naMe"), "Capability 'naMe' not registered");
  CAP_EXPECT_THROW_MSG(registry.get("foo"), "Capability 'foo' not registered");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::add
TEST(CapabilityRegistryTest, add)
{
  CapabilityRegistry registry;
  EXPECT_EQ(registry.getRegistry().size(), 0);

  // Add success
  const auto & capability = registry.add("name", false, "doc");
  EXPECT_EQ(registry.getRegistry().size(), 1);
  EXPECT_EQ(&registry.getRegistry().at("name"), &capability);

  // Add again with same value, return original
  registry.add("name", false, "doc");
  EXPECT_EQ(registry.getRegistry().size(), 1);
  EXPECT_EQ(&registry.getRegistry().at("name"), &capability);

  // Add again, different value
  CAP_EXPECT_THROW_MSG(registry.add("name", true, "doc"),
                       "Capability 'name' already exists and is not equal");
  CAP_EXPECT_THROW_MSG(registry.add("name", 1, "doc"),
                       "Capability 'name' already exists and is not equal");
  CAP_EXPECT_THROW_MSG(registry.add("name", "foo", "doc"),
                       "Capability 'name' already exists and is not equal");
  CAP_EXPECT_THROW_MSG(registry.add("name", false, "doc2"),
                       "Capability 'name' already exists and is not equal");

  // Add a second
  const auto & capability2 = registry.add("name2", true, "doc");
  EXPECT_EQ(registry.getRegistry().size(), 2);
  EXPECT_EQ(&registry.getRegistry().at("name2"), &capability2);
}

class CapabilitiesTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Setup a temporary registry for testing
    std::swap(Moose::Capabilities::getCapabilityRegistry()._capability_registry,
              _old_capability_registry);
    _capability_registry = &Moose::Capabilities::getCapabilityRegistry()._capability_registry;
  }

  void TearDown() override
  {
    // Replace temporary registry for testing with real one
    std::swap(Moose::Capabilities::getCapabilityRegistry()._capability_registry,
              _old_capability_registry);
    _old_capability_registry.clear();
    _capability_registry = nullptr;
  }

  /// Pointer to the current registry during a test
  CapabilityRegistry * _capability_registry = nullptr;
  /// Temporary storage for the actual registry during tests
  CapabilityRegistry _old_capability_registry;
};

/// Test Capabilities::add
TEST_F(CapabilitiesTest, add)
{
  auto & capabilities = Moose::Capabilities::getCapabilityRegistry();

  // success
  const auto & capability = capabilities.add("name", false, "doc");
  EXPECT_EQ(_capability_registry->query("name"), &capability);

  // already exists
  CAP_EXPECT_THROW_MSG(capabilities.add("name", true, "doc"),
                       "Capability 'name' already exists and is not equal");

  // reserved name
  for (const auto & name : Moose::Capabilities::reserved_augmented_capabilities)
    CAP_EXPECT_THROW_MSG(capabilities.add(name, true, "doc"),
                         "The capability \"" + name +
                             "\" is reserved and may not be registered by an application.");
}

/// Test MooseApp::addCapability
TEST_F(CapabilitiesTest, mooseAppAddCapability)
{
  // success
  const auto capability = &MooseApp::addCapability("name", false, "doc");
  EXPECT_EQ(_capability_registry->query("name"), capability);

  // catch any exceptions and report as a mooseError
  EXPECT_MOOSEERROR_MSG(
      MooseApp::addCapability("name", true, "doc"),
      "MooseApp::addCapability(): Capability 'name' already exists and is not equal");
}

/// Test Capabilities::dump
TEST_F(CapabilitiesTest, dump)
{
  auto & capabilities = Moose::Capabilities::getCapabilityRegistry();

  capabilities.add("false", false, "false");
  capabilities.add("true", true, "true");
  capabilities.add("int", 1, "1");
  capabilities.add("int_explicit", 1, "1").setExplicit();
  capabilities.add("string", "string", "string");
  capabilities.add("string_explicit", "string_explicit", "string_explicit").setExplicit();
  capabilities.add("string_enum", "string_enum", "string_enum")
      .setEnumeration({"string_enum", "foo"});
  capabilities.add("string_explicit_enum", "string_explicit_enum", "string_explicit_enum")
      .setExplicit()
      .setEnumeration({"string_explicit_enum", "foo"});

  const auto dumped = capabilities.dump();
  const auto loaded = nlohmann::json::parse(dumped);

  const nlohmann::json expected = {
      {"false", {{"doc", "false"}, {"value", false}}},
      {"int", {{"doc", "1"}, {"explicit", false}, {"value", 1}}},
      {"int_explicit", {{"doc", "1"}, {"explicit", true}, {"value", 1}}},
      {"string", {{"doc", "string"}, {"explicit", false}, {"value", "string"}}},
      {"string_enum",
       {{"doc", "string_enum"},
        {"enumeration", {"string_enum", "foo"}},
        {"explicit", false},
        {"value", "string_enum"}}},
      {"string_explicit",
       {{"doc", "string_explicit"}, {"explicit", true}, {"value", "string_explicit"}}},
      {"string_explicit_enum",
       {{"doc", "string_explicit_enum"},
        {"enumeration", {"string_explicit_enum", "foo"}},
        {"explicit", true},
        {"value", "string_explicit_enum"}}},
      {"true", {{"doc", "true"}, {"value", true}}}};

  EXPECT_EQ(loaded, expected);
}

/// Test Capabilities::check
TEST_F(CapabilitiesTest, check)
{
  auto & capabilities = Moose::Capabilities::getCapabilityRegistry();

  capabilities.add("name", true, "false");
  EXPECT_EQ(std::get<0>(capabilities.check("name")), CERTAIN_PASS);
  EXPECT_EQ(std::get<0>(capabilities.check("!name")), CERTAIN_FAIL);
}

/// Test Capabilities::augment
TEST_F(CapabilitiesTest, augment)
{
  auto & capabilities = Moose::Capabilities::getCapabilityRegistry();

  const nlohmann::json to_augment = {
      {"false", {{"doc", "false"}, {"value", false}}},
      {"int", {{"doc", "1"}, {"explicit", false}, {"value", 1}}},
      {"int_explicit", {{"doc", "1"}, {"explicit", true}, {"value", 1}}},
      {"string", {{"doc", "string"}, {"explicit", false}, {"value", "string"}}},
      {"string_enum",
       {{"doc", "string_enum"},
        {"enumeration", {"string_enum", "foo"}},
        {"explicit", false},
        {"value", "string_enum"}}},
      {"string_explicit",
       {{"doc", "string_explicit"}, {"explicit", true}, {"value", "string_explicit"}}},
      {"string_explicit_enum",
       {{"doc", "string_explicit_enum"},
        {"enumeration", {"string_explicit_enum", "foo"}},
        {"explicit", true},
        {"value", "string_explicit_enum"}}},
      {"true", {{"doc", "true"}, {"value", true}}}};

  capabilities.augment(to_augment, {});

  const auto test_capability =
      [this](const std::string & name,
             const CapabilityValue & value,
             const std::string & doc,
             const bool is_explicit = false,
             const std::optional<std::vector<std::string>> & enumeration = {})
  {
    const auto capability_ptr = _capability_registry->query(name);
    EXPECT_NE(capability_ptr, nullptr);
    const auto & capability = *capability_ptr;
    EXPECT_EQ(capability.getName(), name);
    EXPECT_EQ(capability.getValue(), value);
    EXPECT_EQ(capability.getDoc(), doc);
    EXPECT_EQ(capability.getExplicit(), is_explicit);
    if (enumeration)
    {
      auto & cap_enumeration_ptr = capability.getEnumeration();
      EXPECT_TRUE(cap_enumeration_ptr.has_value());
      EXPECT_EQ(*cap_enumeration_ptr, *enumeration);
    }
  };

  test_capability("false", false, "false");
  test_capability("int", 1, "1");
  test_capability("int_explicit", 1, "1", true);
  test_capability("string", "string", "string");
  test_capability("string_enum",
                  "string_enum",
                  "string_enum",
                  false,
                  std::vector<std::string>{"string_enum", "foo"});
  test_capability("string_explicit", "string_explicit", "string_explicit", true);
  test_capability("string_explicit_enum",
                  "string_explicit_enum",
                  "string_explicit_enum",
                  true,
                  std::vector<std::string>{"string_explicit_enum", "foo"});
  test_capability("true", true, "true");
}
