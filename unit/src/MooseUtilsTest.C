//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

// Moose includes
#include "MooseUtils.h"

TEST(MooseUtils, camelCaseToUnderscore)
{
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("Foo"), "foo");
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("FooBar"), "foo_bar");
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("fooBar"), "foo_bar");

  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("FOObar"), "foobar");
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("fooBAR"), "foo_bar");

  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("PhaseFieldApp"), "phase_field_app");
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("XFEMApp"), "xfemapp");

  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("FOO_BAR"), "foo_bar");
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("FoO__BAR"), "fo_o__bar");
}

TEST(MooseUtils, underscoreToCamelCase)
{
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("foo", false), "foo");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("foo_bar", false), "fooBar");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("_foo_bar", false), "FooBar");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("_foo_bar_", false), "FooBar");

  EXPECT_EQ(MooseUtils::underscoreToCamelCase("foo", true), "Foo");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("foo_bar", true), "FooBar");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("_foo_bar", true), "FooBar");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("_foo_bar_", true), "FooBar");
}

TEST(MooseUtils, toUpper)
{
  std::string specs("RotaryGirder");
  EXPECT_EQ(MooseUtils::toUpper(specs), "ROTARYGIRDER");
}

TEST(MooseUtils, trim)
{
  EXPECT_EQ(MooseUtils::trim("andrew"), "andrew");
  EXPECT_EQ(MooseUtils::trim("   andrew"), "andrew");
  EXPECT_EQ(MooseUtils::trim("andrew    "), "andrew");
  EXPECT_EQ(MooseUtils::trim("      andrew    "), "andrew");
  EXPECT_EQ(MooseUtils::trim("       "), "");
  EXPECT_EQ(MooseUtils::trim(""), "");
  EXPECT_EQ(MooseUtils::trim("andrew edward"), "andrew edward");
  EXPECT_EQ(MooseUtils::trim("      andrew edward"), "andrew edward");
  EXPECT_EQ(MooseUtils::trim("andrew edward    "), "andrew edward");
  EXPECT_EQ(MooseUtils::trim("  andrew edward    "), "andrew edward");
}

TEST(MooseUtils, tokenizeAndConvert)
{
  {
    std::string raw("1,2,3");
    std::vector<Real> tokens;
    MooseUtils::tokenizeAndConvert(raw, tokens, ",");
    EXPECT_EQ(tokens, std::vector<Real>({1, 2, 3}));
  }
  {
    std::string raw("1,  2,     3");
    std::vector<Real> tokens;
    MooseUtils::tokenizeAndConvert(raw, tokens, ",");
    EXPECT_EQ(tokens, std::vector<Real>({1, 2, 3}));
  }
  {
    std::string raw("1    ,2   ,3");
    std::vector<Real> tokens;
    MooseUtils::tokenizeAndConvert(raw, tokens, ",");
    EXPECT_EQ(tokens, std::vector<Real>({1, 2, 3}));
  }
  {
    std::string raw("1    ,      2   ,       3");
    std::vector<Real> tokens;
    MooseUtils::tokenizeAndConvert(raw, tokens, ",");
    EXPECT_EQ(tokens, std::vector<Real>({1, 2, 3}));
  }
}

TEST(MooseUtils, numDigits)
{
  EXPECT_EQ(MooseUtils::numDigits(4), 1);
  EXPECT_EQ(MooseUtils::numDigits(80), 2);
  EXPECT_EQ(MooseUtils::numDigits(812), 3);
  EXPECT_EQ(MooseUtils::numDigits(4512), 4);
  EXPECT_EQ(MooseUtils::numDigits(45434), 5);
  EXPECT_EQ(MooseUtils::numDigits(984345), 6);
  EXPECT_EQ(MooseUtils::numDigits(2454351), 7);
  EXPECT_EQ(MooseUtils::numDigits(14513452), 8);
  EXPECT_EQ(MooseUtils::numDigits(134123454), 9);
  EXPECT_EQ(MooseUtils::numDigits(1513253268), 10);
  EXPECT_EQ(MooseUtils::numDigits(69506060606), 11);
}

TEST(MooseUtils, stringToInteger)
{
  EXPECT_EQ(MooseUtils::stringToInteger("42"), 42);
  EXPECT_EQ(MooseUtils::stringToInteger("-42"), -42);
  EXPECT_THROW(MooseUtils::stringToInteger("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::stringToInteger("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::stringToInteger("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::stringToInteger("42.1", true), std::invalid_argument);
}

TEST(MooseUtils, convertStringInt)
{
  EXPECT_EQ(MooseUtils::convert<short int>("42"), 42);
  EXPECT_EQ(MooseUtils::convert<short int>("-42"), -42);
  EXPECT_THROW(MooseUtils::convert<short int>("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<short int>("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<short int>("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<short int>("42.1", true), std::invalid_argument);

  EXPECT_EQ(MooseUtils::convert<unsigned short int>("42"), 42u);
  EXPECT_THROW(MooseUtils::convert<unsigned short int>("-42", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned short int>("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned short int>("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned short int>("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned short int>("42.1", true), std::invalid_argument);

  EXPECT_EQ(MooseUtils::convert<int>("42"), 42);
  EXPECT_EQ(MooseUtils::convert<int>("-42"), -42);
  EXPECT_THROW(MooseUtils::convert<int>("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<int>("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<int>("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<int>("42.1", true), std::invalid_argument);

  EXPECT_EQ(MooseUtils::convert<unsigned int>("42"), 42u);
  EXPECT_THROW(MooseUtils::convert<unsigned int>("-42", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned int>("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned int>("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned int>("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned int>("42.1", true), std::invalid_argument);

  EXPECT_EQ(MooseUtils::convert<long int>("42"), 42l);
  EXPECT_EQ(MooseUtils::convert<long int>("-42"), -42l);
  EXPECT_THROW(MooseUtils::convert<long int>("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<long int>("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<long int>("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<long int>("42.1", true), std::invalid_argument);

  EXPECT_EQ(MooseUtils::convert<unsigned long int>("42"), 42ul);
  EXPECT_THROW(MooseUtils::convert<unsigned long int>("-42", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned long int>("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned long int>("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned long int>("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned long int>("42.1", true), std::invalid_argument);

  EXPECT_EQ(MooseUtils::convert<long long int>("42"), 42ll);
  EXPECT_EQ(MooseUtils::convert<long long int>("-42"), -42ll);
  EXPECT_THROW(MooseUtils::convert<long long int>("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<long long int>("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<long long int>("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<long long int>("42.1", true), std::invalid_argument);

  EXPECT_EQ(MooseUtils::convert<unsigned long long int>("42"), 42ull);
  EXPECT_EQ(MooseUtils::convert<unsigned long long int>("18446744073709551614"),
            18446744073709551614ull);
  EXPECT_THROW(MooseUtils::convert<unsigned long long int>("-42", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned long long int>("", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned long long int>("42 ", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned long long int>("42foo", true), std::invalid_argument);
  EXPECT_THROW(MooseUtils::convert<unsigned long long int>("42.1", true), std::invalid_argument);

  // Test scientific notation
  EXPECT_EQ(MooseUtils::convert<int>("-1.2e5"), -120000);
  EXPECT_EQ(MooseUtils::convert<unsigned int>("1.2e5"), 120000u);
  EXPECT_EQ(MooseUtils::convert<long int>("-1.2e5"), -120000l);
  EXPECT_EQ(MooseUtils::convert<unsigned long int>("1.2e5"), 120000ul);
  EXPECT_EQ(MooseUtils::convert<long long int>("-1.2e5"), -120000ll);
  EXPECT_EQ(MooseUtils::convert<unsigned long long int>("1.2e5"), 120000ull);

  // Test scientific failure
  EXPECT_THROW(MooseUtils::convert<unsigned int>("1e10", true), std::invalid_argument);

  // Should work with bigger type
  EXPECT_EQ(MooseUtils::convert<unsigned long int>("1e10", true), 10000000000ul);
}

struct TestCase
{
  std::string a;
  std::string b;
  int dist;
};

TEST(MooseUtilsTests, levenshteinDist)
{
  TestCase cases[] = {
      {"hello", "hell", 1},
      {"flood", "foods", 2},
      {"fandango", "odanget", 5},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(TestCase); i++)
  {
    auto test = cases[i];
    int got = MooseUtils::levenshteinDist(test.a, test.b);
    EXPECT_EQ(test.dist, got) << "case " << i + 1 << " FAILED: a=" << test.a << ", b=" << test.b;
  }
}

TEST(MooseUtilsTests, linearPartitionItems)
{
  {
    dof_id_type num_local_items;
    dof_id_type local_items_begin;
    dof_id_type local_items_end;

    MooseUtils::linearPartitionItems(5, 3, 1, num_local_items, local_items_begin, local_items_end);

    EXPECT_EQ(num_local_items, 2);
    EXPECT_EQ(local_items_begin, 2);
    EXPECT_EQ(local_items_end, 4);
  }

  {
    dof_id_type num_local_items;
    dof_id_type local_items_begin;
    dof_id_type local_items_end;

    unsigned int total_items = 0;
    for (unsigned int i = 0; i < 16; i++)
    {
      MooseUtils::linearPartitionItems(
          120, 16, i, num_local_items, local_items_begin, local_items_end);

      total_items += num_local_items;
    }

    EXPECT_EQ(total_items, 120);
  }
}

TEST(MooseUtilsTests, linearPartitionChunk)
{
  // Easy ones first
  EXPECT_EQ(MooseUtils::linearPartitionChunk(4, 2, 0), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(4, 2, 1), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(4, 2, 2), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(4, 2, 3), 1);

  // One leftover
  EXPECT_EQ(MooseUtils::linearPartitionChunk(4, 3, 0), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(4, 3, 1), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(4, 3, 2), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(4, 3, 3), 2);

  EXPECT_EQ(MooseUtils::linearPartitionChunk(8, 3, 0), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(8, 3, 1), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(8, 3, 2), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(8, 3, 3), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(8, 3, 4), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(8, 3, 5), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(8, 3, 6), 2);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(8, 3, 7), 2);

  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 0), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 1), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 2), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 3), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 4), 0);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 5), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 6), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 7), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 8), 1);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 9), 2);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 10), 2);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 11), 2);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 12), 2);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 13), 3);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 14), 3);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 15), 3);
  EXPECT_EQ(MooseUtils::linearPartitionChunk(17, 4, 16), 3);
}
