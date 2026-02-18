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

#include "Capability.h"
#include "CapabilityException.h"

using Moose::Capability;

#define CAP_EXPECT_THROW_MSG(statement, message)                                                   \
  EXPECT_THROW_MSG(statement, Moose::CapabilityException, message);

/// Test Capability::Capability() and getter state
TEST(CapabilityTest, construct)
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
    EXPECT_EQ(cap.getValue(), Capability::Value(bool(false)));
    EXPECT_EQ(cap.getDoc(), "doc");
    EXPECT_FALSE(cap.getExplicit());
    ASSERT_NE(cap.queryBoolValue(), nullptr);
    EXPECT_EQ(*cap.queryBoolValue(), false);
    EXPECT_EQ(cap.queryIntValue(), nullptr);
    EXPECT_EQ(cap.queryStringValue(), nullptr);
    EXPECT_TRUE(cap.hasBoolValue());
    EXPECT_FALSE(cap.hasIntValue());
    EXPECT_FALSE(cap.hasStringValue());
    EXPECT_EQ(cap.getBoolValue(), false);
    CAP_EXPECT_THROW_MSG(cap.getIntValue(),
                         "Capability::getIntValue(): Capability name=false is not an integer");
    CAP_EXPECT_THROW_MSG(cap.getStringValue(),
                         "Capability::getStringValue(): Capability name=false is not a string");
    EXPECT_EQ(cap.valueToString(), "false");
    EXPECT_EQ(cap.toString(), "name=false");
  }

  // bool true value
  {
    Capability cap("name", bool(true), "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), Capability::Value(bool(true)));
    EXPECT_EQ(cap.getDoc(), "doc");
    EXPECT_FALSE(cap.getExplicit());
    ASSERT_NE(cap.queryBoolValue(), nullptr);
    EXPECT_EQ(*cap.queryBoolValue(), true);
    EXPECT_EQ(cap.queryIntValue(), nullptr);
    EXPECT_EQ(cap.queryStringValue(), nullptr);
    EXPECT_TRUE(cap.hasBoolValue());
    EXPECT_FALSE(cap.hasIntValue());
    EXPECT_FALSE(cap.hasStringValue());
    EXPECT_EQ(cap.getBoolValue(), true);
    CAP_EXPECT_THROW_MSG(cap.getIntValue(),
                         "Capability::getIntValue(): Capability name=true is not an integer");
    CAP_EXPECT_THROW_MSG(cap.getStringValue(),
                         "Capability::getStringValue(): Capability name=true is not a string");
    EXPECT_EQ(cap.valueToString(), "true");
    EXPECT_EQ(cap.toString(), "name=true");
  }

  // int value
  {
    Capability cap("name", int(1), "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), Capability::Value(int(1)));
    EXPECT_EQ(cap.getDoc(), "doc");
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_EQ(cap.queryBoolValue(), nullptr);
    ASSERT_NE(cap.queryIntValue(), nullptr);
    EXPECT_EQ(*cap.queryIntValue(), 1);
    EXPECT_EQ(cap.queryStringValue(), nullptr);
    EXPECT_FALSE(cap.hasBoolValue());
    EXPECT_TRUE(cap.hasIntValue());
    EXPECT_FALSE(cap.hasStringValue());
    CAP_EXPECT_THROW_MSG(cap.getBoolValue(),
                         "Capability::getBoolValue(): Capability name=1 is not a bool");
    EXPECT_EQ(cap.getIntValue(), 1);
    CAP_EXPECT_THROW_MSG(cap.getStringValue(),
                         "Capability::getStringValue(): Capability name=1 is not a string");
    EXPECT_EQ(cap.valueToString(), "1");
    EXPECT_EQ(cap.toString(), "name=1");
  }

  // string value
  {
    Capability cap("name", std::string("foo"), "doc");
    EXPECT_EQ(cap.getName(), "name");
    EXPECT_EQ(cap.getValue(), Capability::Value(std::string("foo")));
    EXPECT_EQ(cap.getDoc(), "doc");
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_EQ(cap.queryBoolValue(), nullptr);
    EXPECT_EQ(cap.queryIntValue(), nullptr);
    ASSERT_NE(cap.queryStringValue(), nullptr);
    EXPECT_EQ(*cap.queryStringValue(), "foo");
    EXPECT_FALSE(cap.hasBoolValue());
    EXPECT_FALSE(cap.hasIntValue());
    EXPECT_TRUE(cap.hasStringValue());
    CAP_EXPECT_THROW_MSG(cap.getBoolValue(),
                         "Capability::getBoolValue(): Capability name=foo is not a bool");
    CAP_EXPECT_THROW_MSG(cap.getIntValue(),
                         "Capability::getIntValue(): Capability name=foo is not an integer");
    EXPECT_EQ(cap.getStringValue(), "foo");
    EXPECT_EQ(cap.valueToString(), "foo");
    EXPECT_EQ(cap.toString(), "name=foo");
  }
}

/// Test Capability::Capability enumeration state
TEST(CapabilityTest, enumeration)
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
    const std::set<std::string> enumeration{"foo", "bar"};
    Capability cap("name", std::string("foo"), "doc");

    // initial set
    cap.setEnumeration(enumeration);
    ASSERT_TRUE(cap.queryEnumeration().has_value());
    EXPECT_EQ(*cap.queryEnumeration(), enumeration);
    EXPECT_TRUE(cap.hasEnumeration("foo"));
    EXPECT_TRUE(cap.hasEnumeration("bar"));
    EXPECT_FALSE(cap.hasEnumeration("baz"));

    // enumeration to string
    EXPECT_EQ(cap.enumerationToString(), "bar, foo");

    // setting again ignores checks
    cap.setEnumeration(enumeration);
    auto enumeration_ptr = cap.queryEnumeration();
    ASSERT_TRUE(cap.queryEnumeration().has_value());
    EXPECT_EQ(*cap.queryEnumeration(), enumeration);

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

  // value not in enumeration
  CAP_EXPECT_THROW_MSG(
      Capability("name", std::string("foo"), "doc").setEnumeration({"bar"}),
      "Capability::setEnumeration(): Capability name=foo value not within enumeration");

  // getting enumeration for bool capability, empty
  EXPECT_FALSE(Capability("name", bool(false), "doc").queryEnumeration().has_value());
  // getting enumeration for an int capability
  EXPECT_FALSE(Capability("name", int(1), "doc").queryEnumeration().has_value());
  // enumerationToString() without an enumeration
  CAP_EXPECT_THROW_MSG(
      Capability("name", std::string("foo"), "doc").enumerationToString(),
      "Capability::enumerationToString(): Capability 'name' does not have an enumeration");
}

/// Test Capability::Capability explicit state
TEST(CapabilityTest, explicitState)
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

/// Test Capability::Capability::negateValue()
TEST(CapabilityTest, negateValue)
{
  // negate bool value that is already false
  {
    Capability cap("name", bool(false), "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), Capability::Value{false});
  }

  // negate bool value that is not false
  {
    Capability cap("name", bool(true), "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), Capability::Value{false});
  }

  // negate int value
  {
    Capability cap("name", int(1), "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), Capability::Value{false});
  }

  // negate int value that is explicit
  {
    Capability cap("name", int(1), "doc");
    cap.setExplicit();
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), Capability::Value{false});
  }

  // string value
  {
    Capability cap("name", std::string("foo"), "doc");
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), Capability::Value{false});
  }

  // string value that is explicit
  {
    Capability cap("name", std::string("foo"), "doc");
    cap.setExplicit();
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), Capability::Value{false});
  }

  // string value that has an enumeration
  {
    Capability cap("name", std::string("foo"), "doc");
    cap.setEnumeration({"foo"});
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), Capability::Value{false});
  }

  // string value that has an enumeration and is explicit
  {
    Capability cap("name", std::string("foo"), "doc");
    cap.setExplicit();
    cap.setEnumeration({"foo"});
    cap.negateValue();
    EXPECT_FALSE(cap.getExplicit());
    EXPECT_FALSE(cap._enumeration.has_value());
    EXPECT_EQ(cap.getValue(), Capability::Value{false});
  }
}
