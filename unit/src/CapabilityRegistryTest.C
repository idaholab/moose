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

#include "CapabilityException.h"
#include "CapabilityRegistry.h"

#include "nlohmann/json.h"

using Moose::internal::CapabilityRegistry;
using CheckState = Moose::internal::CapabilityRegistry::CheckState;

#define CAP_EXPECT_THROW_MSG(statement, message)                                                   \
  EXPECT_THROW_MSG(statement, Moose::CapabilityException, message);

#define CAP_CHECK_EXPECT_EQ(requirement, check_state)                                              \
  EXPECT_EQ(registry.check(requirement).state, check_state)

#define CAP_CHECK_EXPECT_ERROR(requirement, error)                                                 \
  CAP_EXPECT_THROW_MSG(registry.check(requirement), error)

/// Test CapabilityRegistry::check for bool capabilities
TEST(CapabilityRegistryTest, checkBool)
{
  CapabilityRegistry registry;
  registry.add("bool", bool(true), "Boolean test capability");
  registry.add("bool2", bool(false), "Boolean test capability 2");

  CAP_CHECK_EXPECT_EQ("bool", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("!bool", CheckState::CERTAIN_FAIL);
  CAP_CHECK_EXPECT_EQ("!bool2", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("bool2", CheckState::CERTAIN_FAIL);
  CAP_CHECK_EXPECT_EQ("noexist", CheckState::POSSIBLE_FAIL);
  CAP_CHECK_EXPECT_EQ("!noexist", CheckState::POSSIBLE_PASS);

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

/// Test CapabilityRegistry::checkfor int capabilities
TEST(CapabilityRegistryTest, checkInt)
{
  CapabilityRegistry registry;
  registry.add("int", int(78), "Integer test capability");

  const std::vector<std::string> is_true = {
      "", "=78", "==78", "<=78", ">=78", "<=79", ">=77", "<79", ">77", "!=77", "!=79"};
  const std::vector<std::string> is_false = {"!=78", ">=79", "<=77", "<78", ">78", "==77"};

  for (const auto & c : is_true)
  {
    CAP_CHECK_EXPECT_EQ("int" + c, CheckState::CERTAIN_PASS);
    CAP_CHECK_EXPECT_EQ("!(int" + c + ")", CheckState::CERTAIN_FAIL);
  }
  for (const auto & c : is_false)
  {
    CAP_CHECK_EXPECT_EQ("int" + c, CheckState::CERTAIN_FAIL);
    CAP_CHECK_EXPECT_EQ("!(int" + c + ")", CheckState::CERTAIN_PASS);
  }

  CAP_CHECK_EXPECT_ERROR("int<", "Unable to parse requested capabilities 'int<'.");
  CAP_CHECK_EXPECT_ERROR("int<bla",
                         "Capability statement 'int<bla': capability 'int=78' cannot be "
                         "compared to a string.");
  CAP_CHECK_EXPECT_ERROR("int>1.0",
                         "Capability statement 'int>1.0': capability 'int=78' cannot be "
                         "compared to a version.");
}

/// Test CapabilityRegistry::check for explicit int capabilities
TEST(CapabilityRegistryTest, checkIntExplicit)
{
  CapabilityRegistry registry;
  registry.add("int", int(78), "Integer test capability").setExplicit();

  // should still allow as explicit
  CAP_CHECK_EXPECT_EQ("int=78", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("int=79", CheckState::CERTAIN_FAIL);

  // not explicit
  CAP_CHECK_EXPECT_ERROR("int",
                         "Capability statement 'int': capability 'int' requires a value and "
                         "cannot be used in a boolean expression");
  CAP_CHECK_EXPECT_ERROR("!int",
                         "Capability statement 'int': capability 'int' requires a value and "
                         "cannot be used in a boolean expression");
}

/// Test CapabilityRegistry::check for string capabilities
TEST(CapabilityRegistryTest, checkString)
{
  CapabilityRegistry registry;
  registry.add("string", std::string("clang"), "String test capability");

  CAP_CHECK_EXPECT_EQ("string", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("!string", CheckState::CERTAIN_FAIL);

  CAP_CHECK_EXPECT_EQ("string=clang", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("string=CLANG", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("string=gcc", CheckState::CERTAIN_FAIL);

  CAP_CHECK_EXPECT_ERROR("string>", "Unable to parse requested capabilities 'string>'.");
  CAP_CHECK_EXPECT_ERROR("string>0",
                         "Capability statement 'string>0': capability 'string=clang' cannot be "
                         "compared to a version.");
  CAP_CHECK_EXPECT_ERROR("string>1.0",
                         "Capability statement 'string>1.0': capability 'string=clang' cannot be "
                         "compared to a version.");
}

/// Test CapabilityRegistry::check for enumerated string capabilities
TEST(CapabilityRegistryTest, checkEnumeratedString)
{
  CapabilityRegistry registry;
  registry.add("string", std::string("clang"), "String test capability")
      .setEnumeration({"clang", "gcc"});

  // should still allow as a bool
  CAP_CHECK_EXPECT_EQ("string", CheckState::CERTAIN_PASS);

  // within the enumeration
  CAP_CHECK_EXPECT_EQ("string=clang", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("string=gcc", CheckState::CERTAIN_FAIL);

  // not within the enumeration
  CAP_CHECK_EXPECT_ERROR("string=foo",
                         "Capability statement 'string=foo': 'foo' invalid for capability "
                         "'string'; valid values: clang, gcc");
}

/// Test CapabilityRegistry::check for explicit string capabilities
TEST(CapabilityRegistryTest, checkExplicitString)
{
  CapabilityRegistry registry;
  registry.add("string", std::string("clang"), "String test capability").setExplicit();

  // should still allow as explicit
  CAP_CHECK_EXPECT_EQ("string=clang", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("string=foo", CheckState::CERTAIN_FAIL);

  // not explicit
  CAP_CHECK_EXPECT_ERROR("string",
                         "Capability statement 'string': capability 'string' requires a value and "
                         "cannot be used in a boolean expression");
  CAP_CHECK_EXPECT_ERROR("!string",
                         "Capability statement 'string': capability 'string' requires a value and "
                         "cannot be used in a boolean expression");
}

/// Test CapabilityRegistry::check for explicit and enumerated string capabilities
TEST(CapabilityRegistryTest, checkExplicitEnumeratedString)
{
  CapabilityRegistry registry;
  registry.add("string", std::string("clang"), "String test capability")
      .setEnumeration({"clang", "gcc"})
      .setExplicit();

  // should still allow as explicit
  CAP_CHECK_EXPECT_EQ("string=clang", CheckState::CERTAIN_PASS);

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

/// Test CapabilityRegistry::check for versioned string capabilities
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
    CAP_CHECK_EXPECT_EQ("version" + c, CheckState::CERTAIN_PASS);
    CAP_CHECK_EXPECT_EQ("!(version" + c + ")", CheckState::CERTAIN_FAIL);
  }
  for (const auto & c : is_false)
  {
    CAP_CHECK_EXPECT_EQ("version" + c, CheckState::CERTAIN_FAIL);
    CAP_CHECK_EXPECT_EQ("!(version" + c + ")", CheckState::CERTAIN_PASS);
  }
  CAP_CHECK_EXPECT_ERROR("!version<", "Unable to parse requested capabilities '!version<'.");
  CAP_CHECK_EXPECT_ERROR("version=foo",
                         "Capability statement 'version=foo': capability 'version=3.2.1' cannot be "
                         "compared to a string.");
}

/// Test CapabilityRegistry::check for multiple valued requirements
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
                      CheckState::POSSIBLE_PASS);
  CAP_CHECK_EXPECT_EQ("!doesnotexist & version<4.2.2 &"
                      "int<100 & unittest2_int>50 & string!=Popel ",
                      CheckState::UNKNOWN);
  CAP_CHECK_EXPECT_EQ("doesnotexist & doesnotexist>2.0.1", CheckState::POSSIBLE_FAIL);
  CAP_CHECK_EXPECT_EQ("!doesnotexist | doesnotexist<=2.0.1", CheckState::POSSIBLE_PASS);
  CAP_CHECK_EXPECT_EQ("bool & int!=78", CheckState::CERTAIN_FAIL);
  CAP_CHECK_EXPECT_EQ("bool | int!=78", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ(" !bool | (string=gcc | version<1.0)", CheckState::CERTAIN_FAIL);
  CAP_CHECK_EXPECT_EQ("(bool & int=78) & (string=clang & version>2.0)", CheckState::CERTAIN_PASS);
  CAP_CHECK_EXPECT_EQ("bool & string_explicit=foo & int_explicit=78", CheckState::CERTAIN_FAIL);
  CAP_CHECK_EXPECT_EQ("(string_enum=gcc | string_enum=clang) & bool", CheckState::CERTAIN_PASS);

  CAP_CHECK_EXPECT_ERROR("bool int< string==Popel",
                         "Unable to parse requested capabilities 'bool int< string==Popel'.");
  CAP_CHECK_EXPECT_ERROR("(bool", "Unable to parse requested capabilities '(bool'.");
}

/// Test CapabilityRegistry::check for empty requirements
TEST(CapabilityRegistryTest, checkEmpty)
{
  CapabilityRegistry registry;
  CAP_CHECK_EXPECT_EQ("", CheckState::CERTAIN_PASS);
}

/// Test CapabilityRegistry::check for a parse failure
TEST(CapabilityRegistryTest, checkParseFail)
{
  CapabilityRegistry registry;
  CAP_CHECK_EXPECT_ERROR("foo bar", "Unable to parse requested capabilities 'foo bar'.");
}

#undef CAP_CHECK_EXPECT_EQ
#undef CAP_CHECK_EXPECT_ERROR

/// Test CapabilityRegistry::query
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

/// Test CapabilityRegistry::get
TEST(CapabilityRegistryTest, get)
{
  CapabilityRegistry registry;
  const auto & capability = registry.add("name", bool(false), "doc");

  // Exact name
  EXPECT_EQ(&registry.get("name"), &capability);
  EXPECT_EQ(&std::as_const(registry).get("name"), &capability);

  // Name to lower
  EXPECT_EQ(&registry.get("naMe"), &capability);
  EXPECT_EQ(&std::as_const(registry).get("naMe"), &capability);

  // Not found
  CAP_EXPECT_THROW_MSG(registry.get("foo"), "Capability 'foo' not registered");
  CAP_EXPECT_THROW_MSG(std::as_const(registry).get("foo"), "Capability 'foo' not registered");
}

/// Test CapabilityRegistry::add
TEST(CapabilityRegistryTest, add)
{
  CapabilityRegistry registry;
  EXPECT_EQ(registry.size(), 0);

  // Add success
  const auto & capability = registry.add("name", bool(false), "doc");
  EXPECT_EQ(registry.size(), 1);
  EXPECT_EQ(&registry.get("name"), &capability);

  // Add again with same value, return original
  registry.add("name", bool(false), "doc");
  EXPECT_EQ(registry.size(), 1);
  EXPECT_EQ(&registry.get("name"), &capability);

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
  EXPECT_EQ(registry.size(), 2);
  EXPECT_EQ(&registry.get("name2"), &capability2);
}
