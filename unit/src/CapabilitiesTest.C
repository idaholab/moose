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

using Moose::Capabilities;
using Moose::CapabilityUtils::augmented_capability_names;
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
  CAP_EXPECT_THROW_MSG(Capability("", bool(true), "doc"), "Capability has empty name");

  // disallowed characters
  CAP_EXPECT_THROW_MSG(
      Capability("A!", bool(true), "doc"),
      "Capability 'A!': Name has unallowed characters; allowed characters = 'a-z, 0-9, _, -'");

  // string value has disallowed characters
  CAP_EXPECT_THROW_MSG(Capability("cap", std::string("A!"), "doc"),
                       "String capability 'cap': value 'A!' has unallowed characters; allowed "
                       "characters = 'a-z, 0-9, _, ., -'");

  // bool false value
  {
    Capability cap("name", bool(false), "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), CapabilityValue(bool(false)));
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
    Capability cap("name", bool(true), "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), CapabilityValue(bool(true)));
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
    Capability cap("name", int(1), "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), CapabilityValue(int(1)));
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
    Capability cap("name", std::string("foo"), "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), CapabilityValue(std::string("foo")));
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
  CAP_EXPECT_THROW_MSG(Capability("name", bool(false), "doc").setEnumeration({}),
                       "Capability::setEnumeration(): Capability 'name' is not string-valued and "
                       "cannot have an enumeration");

  // can't set an enumeration for an int
  CAP_EXPECT_THROW_MSG(Capability("name", int(1), "doc").setEnumeration({}),
                       "Capability::setEnumeration(): Capability 'name' is not string-valued and "
                       "cannot have an enumeration");

  // check set
  {
    const std::vector<std::string> enumeration{"foo", "bar"};
    Capability cap("name", std::string("foo"), "doc");

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
  CAP_EXPECT_THROW_MSG(Capability("name", std::string("foo"), "doc").setEnumeration({}),
                       "Capability::setEnumeration(): Enumeration is empty for 'name'");

  // unallowed characters
  CAP_EXPECT_THROW_MSG(
      Capability("name", std::string("foo"), "doc").setEnumeration({"abc!"}),
      "Capability::setEnumeration(): Enumeration value 'abc!' for capability 'name' "
      "has unallowed characters; allowed characters = 'a-z, 0-9, _, -'");
  CAP_EXPECT_THROW_MSG(
      Capability("name", std::string("foo"), "doc").setEnumeration({"abc", "def!"}),
      "Capability::setEnumeration(): Enumeration value 'def!' for capability 'name' "
      "has unallowed characters; allowed characters = 'a-z, 0-9, _, -'");

  // duplicates
  CAP_EXPECT_THROW_MSG(
      Capability("name", std::string("foo"), "doc").setEnumeration({"foo", "foo"}),
      "Capability::setEnumeration(): Duplicate enumeration 'foo' for capability 'name'");

  // value not in enumeration
  CAP_EXPECT_THROW_MSG(
      Capability("name", std::string("foo"), "doc").setEnumeration({"bar"}),
      "Capability::setEnumeration(): Capability name=foo value not within enumeration");

  // getting enumeration for bool capability
  CAP_EXPECT_THROW_MSG(Capability("name", bool(false), "doc").getEnumeration(),
                       "Capability::getEnumeration(): Capability 'name' is not string-valued and "
                       "cannot have an enumeration");
  // getting enumeration for an int capability
  CAP_EXPECT_THROW_MSG(Capability("name", int(1), "doc").getEnumeration(),
                       "Capability::getEnumeration(): Capability 'name' is not string-valued and "
                       "cannot have an enumeration");
  // enumerationToString() without an enumeration
  CAP_EXPECT_THROW_MSG(
      Capability("name", std::string("foo"), "doc").enumerationToString(),
      "Capability::enumerationToString(): Capability 'name' does not have an enumeration");
}

/// Test Moose::CapabilityUtils::Capability explicit state
TEST(CapabilityTest, capabilityExplicit)
{
  // can't set a bool to be explicit
  CAP_EXPECT_THROW_MSG(
      Capability("name", bool(false), "doc").setExplicit(),
      "Capability::setExplicit(): Capability 'name' is bool-valued and cannot be set as explicit");

  // int explicit capability
  EXPECT_TRUE(Capability("name", int(1), "doc").setExplicit().getExplicit());

  // string explicit capability
  EXPECT_TRUE(Capability("name", std::string("foo"), "doc").setExplicit().getExplicit());
}

/// Test Moose::CapabilityUtils::Capability::negateValue()
TEST(CapabilityTest, capabilityNegateValue)
{
  // negate bool value that is already false
  {
    Capability cap("name", bool(false), "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // negate bool value that is not false
  {
    Capability cap("name", bool(true), "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // negate int value
  {
    Capability cap("name", int(1), "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // negate int value that is explicit
  {
    Capability cap("name", int(1), "doc");
    cap.setExplicit();
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // string value
  {
    Capability cap("name", std::string("foo"), "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // string value that is explicit
  {
    Capability cap("name", std::string("foo"), "doc");
    cap.setExplicit();
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // string value that has an enumeration
  {
    Capability cap("name", std::string("foo"), "doc");
    cap.setEnumeration({"foo"});
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), CapabilityValue{false});
  }

  // string value that has an enumeration and is explicit
  {
    Capability cap("name", std::string("foo"), "doc");
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
  registry.add("bool", bool(true), "Boolean test capability");
  registry.add("bool2", bool(false), "Boolean test capability 2");

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
  CAP_CHECK_EXPECT_ERROR("bool>1",
                         "Capability statement 'bool>1': capability 'bool=true' cannot be "
                         "compared to a number.")
  CAP_CHECK_EXPECT_ERROR("bool>1.1",
                         "Capability statement 'bool>1.1': capability 'bool=true' cannot be "
                         "compared to a version number.")
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::checkfor int capabilities
TEST(CapabilityRegistryTest, checkInt)
{
  CapabilityRegistry registry;
  registry.add("int", int(78), "Integer test capability");

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
  registry.add("int", int(78), "Integer test capability").setExplicit();

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
  registry.add("string", std::string("clang"), "String test capability");

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
  registry.add("string", std::string("clang"), "String test capability")
      .setEnumeration({"clang", "gcc"});

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
  registry.add("string", std::string("clang"), "String test capability").setExplicit();

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
  registry.add("string", std::string("clang"), "String test capability")
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
  registry.add("version", std::string("3.2.1"), "Version number test capability");

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
  CAP_CHECK_EXPECT_ERROR("version=foo",
                         "Capability statement 'version=foo': capability 'version=3.2.1' cannot be "
                         "compared to a string.");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::check for multiple valued requirements
TEST(CapabilityRegistryTest, checkMultiple)
{
  CapabilityRegistry registry;
  registry.add("bool", bool(true), "Multiple capability test bool");
  registry.add("int", int(78), "Multiple capability test int");
  registry.add("int_explicit", int(79), "Multiple capability test int explicit");
  registry.add("string", std::string("clang"), "Multiple capability test string");
  registry.add("string_explicit", std::string("foo"), "Multiple capability test string explicit")
      .setExplicit();
  registry.add("string_enum", std::string("clang"), "Multiple capability test string enum")
      .setEnumeration({"clang", "gcc"});
  registry.add("version", std::string("3.2.1"), "Multiple capability test version number");

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
  const auto & capability = registry.add("name", bool(false), "doc");

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
  const auto & capability = registry.add("name", bool(false), "doc");

  EXPECT_EQ(&registry.get("name"), &capability);
  EXPECT_EQ(&std::as_const(registry).get("name"), &capability);
  EXPECT_EQ(&registry.get("naMe"), &capability);
  EXPECT_EQ(&std::as_const(registry).get("naMe"), &capability);
  CAP_EXPECT_THROW_MSG(registry.get("foo"), "Capability 'foo' not registered");
  CAP_EXPECT_THROW_MSG(std::as_const(registry).get("foo"), "Capability 'foo' not registered");
}

/// Test Moose::CapabilityUtils::CapabilityRegistry::add
TEST(CapabilityRegistryTest, add)
{
  CapabilityRegistry registry;
  EXPECT_EQ(registry.getRegistry().size(), 0);

  // Add success
  const auto & capability = registry.add("name", bool(false), "doc");
  EXPECT_EQ(registry.getRegistry().size(), 1);
  EXPECT_EQ(&registry.getRegistry().at("name"), &capability);

  // Add again with same value, return original
  registry.add("name", bool(false), "doc");
  EXPECT_EQ(registry.getRegistry().size(), 1);
  EXPECT_EQ(&registry.getRegistry().at("name"), &capability);

  // Add again, different value
  CAP_EXPECT_THROW_MSG(registry.add("name", bool(true), "doc"),
                       "Capability 'name' already exists and is not equal");
  CAP_EXPECT_THROW_MSG(registry.add("name", int(1), "doc"),
                       "Capability 'name' already exists and is not equal");
  CAP_EXPECT_THROW_MSG(registry.add("name", std::string("foo"), "doc"),
                       "Capability 'name' already exists and is not equal");
  CAP_EXPECT_THROW_MSG(registry.add("name", bool(false), "doc2"),
                       "Capability 'name' already exists and is not equal");

  // Add a second
  const auto & capability2 = registry.add("name2", bool(true), "doc");
  EXPECT_EQ(registry.getRegistry().size(), 2);
  EXPECT_EQ(&registry.getRegistry().at("name2"), &capability2);
}

class CapabilitiesTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Setup a temporary registry for testing
    std::swap(Capabilities::getCapabilities()._capability_registry, _old_capability_registry);
    _capability_registry = &Capabilities::getCapabilities()._capability_registry;
    ASSERT_TRUE(Capabilities::getCapabilities()._capability_registry.getRegistry().empty());
  }

  void TearDown() override
  {
    // Replace temporary registry for testing with real one
    std::swap(Capabilities::getCapabilities()._capability_registry, _old_capability_registry);
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
  auto & capabilities = Capabilities::getCapabilities();

  // success
  const auto & capability = capabilities.add("name", bool(false), "doc");
  EXPECT_EQ(_capability_registry->query("name"), &capability);

  // already exists
  EXPECT_MOOSEERROR_MSG(capabilities.add("name", bool(true), "doc"),
                        "Capability 'name' already exists and is not equal");

  // reserved name
  for (const auto & name : augmented_capability_names)
    EXPECT_MOOSEERROR_MSG(capabilities.add(name, bool(true), "doc"),
                          "The capability \"" + name +
                              "\" is reserved and may not be registered by an application.");
}

/// Test Capabilities::query
TEST_F(CapabilitiesTest, query)
{
  auto & capabilities = Capabilities::getCapabilities();
  const auto & capability = capabilities.add("name", bool(false), "doc");

  EXPECT_EQ(capabilities.query("name"), &capability);
  EXPECT_EQ(capabilities.query("naMe"), &capability);
  EXPECT_EQ(capabilities.query("foo"), nullptr);
}

/// Test Capabilities::get
TEST_F(CapabilitiesTest, get)
{
  auto & capabilities = Capabilities::getCapabilities();
  const auto & capability = capabilities.add("name", bool(false), "doc");

  EXPECT_EQ(&capabilities.get("name"), &capability);
  EXPECT_EQ(&capabilities.get("naMe"), &capability);
  EXPECT_MOOSEERROR_MSG(capabilities.get("foo"),
                        "Capabilities::get(): Capability 'foo' not registered");
}

/// Test MooseApp::addBoolCapability
TEST_F(CapabilitiesTest, mooseAppAddBoolCapability)
{
  // success
  const auto capability = &MooseApp::addBoolCapability("name", false, "doc");
  EXPECT_EQ(_capability_registry->query("name"), capability);

  // exceptions are moose errors
  EXPECT_MOOSEERROR_MSG(MooseApp::addBoolCapability("name", true, "doc"),
                        "Capability 'name' already exists and is not equal");
}

/// Test MooseApp::addIntCapability
TEST_F(CapabilitiesTest, mooseAppAddIntCapability)
{
  // success
  const auto capability = &MooseApp::addIntCapability("name", 1, "doc");
  EXPECT_EQ(_capability_registry->query("name"), capability);

  // catch any exceptions and report as a mooseError
  EXPECT_MOOSEERROR_MSG(MooseApp::addIntCapability("name", 2, "doc"),
                        "Capability 'name' already exists and is not equal");
}

/// Test MooseApp::addStringCapability
TEST_F(CapabilitiesTest, mooseAppAddStringCapability)
{
  // success
  const auto capability = &MooseApp::addStringCapability("name", "value", "doc");
  EXPECT_EQ(_capability_registry->query("name"), capability);

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
    EXPECT_EQ(_capability_registry->query("name"), capability);
  }

  // adding the second time gives no deprecation warning
  {
    Moose::ScopedDeprecatedIsError deprecated_is_error(true);
    MooseApp::addCapability("name", "value", "doc");
    EXPECT_EQ(_capability_registry->query("name"), capability);
  }
}

/// Test Capabilities::dump
TEST_F(CapabilitiesTest, dump)
{
  auto & capabilities = Capabilities::getCapabilities();

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
  auto & capabilities = Capabilities::getCapabilities();

  capabilities.add("name", bool(true), "false");
  EXPECT_EQ(std::get<0>(capabilities.check("name")), CERTAIN_PASS);
  EXPECT_EQ(std::get<0>(capabilities.check("!name")), CERTAIN_FAIL);
}

/// Test Capabilities::augment
TEST_F(CapabilitiesTest, augment)
{
  auto & capabilities = Capabilities::getCapabilities();

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

  test_capability("false", bool(false), "false");
  test_capability("int", int(1), "1");
  test_capability("int_explicit", int(1), "1", true);
  test_capability("string", std::string("string"), "string");
  test_capability("string_enum",
                  std::string("string_enum"),
                  "string_enum",
                  false,
                  std::vector<std::string>{"string_enum", "foo"});
  test_capability("string_explicit", std::string("string_explicit"), "string_explicit", true);
  test_capability("string_explicit_enum",
                  std::string("string_explicit_enum"),
                  "string_explicit_enum",
                  true,
                  std::vector<std::string>{"string_explicit_enum", "foo"});
  test_capability("true", bool(true), "true");
}
