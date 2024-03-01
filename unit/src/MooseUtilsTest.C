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
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

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

TEST(MooseUtils, removeExtraWhitespace)
{
  const auto test = [](const std::string & str, const std::string & expected_result)
  {
    const auto result = MooseUtils::removeExtraWhitespace(str);
    ASSERT_EQ(expected_result, result);
  };

  test(" foo", "foo");
  test("foo ", "foo");
  test(" foo ", "foo");
  test("  foo  ", "foo");
  test("      a b  c d", "a b c d");
  test("", "");
  test(" ", "");
  test("    ", "");
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
  EXPECT_EQ(MooseUtils::convert<unsigned long long int>("18446744073709551615"),
            18446744073709551615ull);
  EXPECT_EQ(MooseUtils::convert<unsigned long long int>("18446744073709551614"),
            18446744073709551614ull);
  EXPECT_EQ(MooseUtils::convert<unsigned long long int>("18446744073709551613"),
            18446744073709551613ull);
  EXPECT_EQ(MooseUtils::convert<unsigned long long int>("17446744073709551600"),
            17446744073709551600ull);
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

TEST(MooseUtils, split)
{
  std::vector<std::string> out = MooseUtils::split("foo;bar;foobar", ";");
  EXPECT_EQ(out, std::vector<std::string>({"foo", "bar", "foobar"}));

  out = MooseUtils::split(";foo;bar", ";");
  EXPECT_EQ(out, std::vector<std::string>({"", "foo", "bar"}));

  out = MooseUtils::split("foo;;bar", ";");
  EXPECT_EQ(out, std::vector<std::string>({"foo", "", "bar"}));

  out = MooseUtils::split("foo;bar;", ";");
  EXPECT_EQ(out, std::vector<std::string>({"foo", "bar", ""}));

  out = MooseUtils::split("foo;bar;;", ";");
  EXPECT_EQ(out, std::vector<std::string>({"foo", "bar", "", ""}));

  out = MooseUtils::split("a/b/c/d", "/", 2);
  EXPECT_EQ(out, std::vector<std::string>({"a", "b", "c/d"}));

  out = MooseUtils::split("", ";");
  EXPECT_EQ(out, std::vector<std::string>({""}));
}

TEST(MooseUtils, rsplit)
{
  std::vector<std::string> out = MooseUtils::rsplit("foo;bar;foobar", ";");
  EXPECT_EQ(out, std::vector<std::string>({"foo", "bar", "foobar"}));

  out = MooseUtils::rsplit(";foo;bar", ";");
  EXPECT_EQ(out, std::vector<std::string>({"", "foo", "bar"}));

  out = MooseUtils::rsplit("foo;;bar", ";");
  EXPECT_EQ(out, std::vector<std::string>({"foo", "", "bar"}));

  out = MooseUtils::rsplit("foo;bar;", ";");
  EXPECT_EQ(out, std::vector<std::string>({"foo", "bar", ""}));

  out = MooseUtils::rsplit("foo;bar;;", ";");
  EXPECT_EQ(out, std::vector<std::string>({"foo", "bar", "", ""}));

  out = MooseUtils::rsplit("a/b/c/d", "/", 2);
  EXPECT_EQ(out, std::vector<std::string>({"a/b", "c", "d"}));
}

TEST(MooseUtils, join)
{
  std::vector<std::string> str = {"foo", "bar", "foobar"};
  EXPECT_EQ(MooseUtils::join(str, ";"), "foo;bar;foobar");

  str = {"", "foo", "bar", "foobar"};
  EXPECT_EQ(MooseUtils::join(str, ";"), ";foo;bar;foobar");

  str = {"foo", "bar", "foobar", ""};
  EXPECT_EQ(MooseUtils::join(str, ";"), "foo;bar;foobar;");

  str = {"foo", "bar", "foobar", "", ""};
  EXPECT_EQ(MooseUtils::join(str, ";"), "foo;bar;foobar;;");
}

TEST(MooseUtils, hostname)
{
  // ok as long mooseError is not triggered
  MooseUtils::hostname();
}

TEST(MooseUtils, symlink)
{
  // ok as long mooseError is not triggered
  MooseUtils::createSymlink("data/example_file", "testlink");
  MooseUtils::clearSymlink("testlink");
}

TEST(MooseUtils, fileSize) { EXPECT_EQ(MooseUtils::fileSize("data/example_file"), 92); }

TEST(MooseUtils, realpath)
{
  // ok as long mooseError is not triggered
  MooseUtils::realpath("data/example_file");
}

TEST(MooseUtils, directory)
{
  std::string path;

  path = "a/b/c";
  MooseUtils::makedirs(path);
  EXPECT_TRUE(MooseUtils::pathExists(path));
  MooseUtils::removedirs(path);
  EXPECT_FALSE(MooseUtils::pathExists(path));

  // mkdir for an existing directory
  path = "a/b/c";
  MooseUtils::makedirs(path);
  MooseUtils::makedirs(path);
  EXPECT_TRUE(MooseUtils::pathExists(path));
  MooseUtils::removedirs(path);
  EXPECT_FALSE(MooseUtils::pathExists(path));
  MooseUtils::removedirs(path);
  EXPECT_FALSE(MooseUtils::pathExists(path));

  // test ..
  path = "no_dir_name_like_this/../../b/c";
  MooseUtils::makedirs(path);
  EXPECT_TRUE(MooseUtils::pathExists("../b/c"));
  MooseUtils::removedirs(path);
  EXPECT_FALSE(MooseUtils::pathExists("../b/c"));
  EXPECT_FALSE(MooseUtils::pathExists(path));

  // test .
  path = "./b/c";
  MooseUtils::makedirs(path);
  EXPECT_TRUE(MooseUtils::pathExists("b/c"));
  MooseUtils::removedirs(path);
  EXPECT_FALSE(MooseUtils::pathExists("b/c"));
  EXPECT_FALSE(MooseUtils::pathExists(path));

  // test absolute path
  path = "a/b/c";
  MooseUtils::makedirs(path);
  std::string rpath = MooseUtils::realpath(path);
  MooseUtils::removedirs(path);
  MooseUtils::makedirs(rpath);
  EXPECT_TRUE(MooseUtils::pathExists(path));
  MooseUtils::removedirs(rpath);
  EXPECT_FALSE(MooseUtils::pathExists(path));
  EXPECT_THROW(MooseUtils::makedirs("/should_not_access", true), std::invalid_argument);
}

TEST(MooseUtils, globCompare)
{
  EXPECT_TRUE(MooseUtils::globCompare("Bell", "?ell"));
  EXPECT_TRUE(MooseUtils::globCompare("Bovine", "*vin*"));
  EXPECT_TRUE(MooseUtils::globCompare("Bathroom", "Ba*"));
  EXPECT_TRUE(MooseUtils::globCompare("Mop", "M*op"));
  EXPECT_TRUE(MooseUtils::globCompare("Four", "????"));
  EXPECT_TRUE(MooseUtils::globCompare("Bathroom", "*room"));
  EXPECT_TRUE(MooseUtils::globCompare("Irradiation", "*ra*o*"));
  EXPECT_TRUE(MooseUtils::globCompare("Clock", "C*ck"));
  EXPECT_TRUE(MooseUtils::globCompare("LoveIsInTheAirTonight", "LoveIsInThe???Tonight"));
  EXPECT_FALSE(MooseUtils::globCompare("Moose", "*ealII*"));
  EXPECT_FALSE(MooseUtils::globCompare("FEM", "?EN"));
  EXPECT_FALSE(MooseUtils::globCompare("Three", "????"));
}

TEST(MooseUtils, SemidynamicVector)
{
  // uninitialized storage
  MooseUtils::SemidynamicVector<int, 10, false> test(4);
  EXPECT_EQ(test.size(), 4);
  EXPECT_EQ(test.max_size(), 10);

  // test iterator
  unsigned int count = 0;
  for (auto & i : test)
    i = ++count;
  EXPECT_EQ(count, 4);

  // test resize
  test.resize(6);
  count = 0;
  for (auto & i : test)
    i = ++count;
  EXPECT_EQ(count, 6);

  // test const_iterator
  const auto & ctest = test;
  count = 0;
  for (auto & i : ctest)
    count += i;
  EXPECT_EQ(count, 1 + 2 + 3 + 4 + 5 + 6);

  // test push back
  test.push_back(100);
  count = 0;
  for (auto & i : ctest)
    count += i;
  EXPECT_EQ(count, 1 + 2 + 3 + 4 + 5 + 6 + 100);

  // test emplace_back
  test.emplace_back(200);
  count = 0;
  for (auto & i : ctest)
    count += i;
  EXPECT_EQ(count, 1 + 2 + 3 + 4 + 5 + 6 + 100 + 200);
}

struct Custom
{
  Custom() : _data(333) {}
  int _data;
};

TEST(MooseUtils, SemidynamicVectorInit)
{
  // value initialized storage (default equivalent to ...<int, 1000>)
  MooseUtils::SemidynamicVector<int, 1000, true> test(1000);
  for (auto & i : test)
    EXPECT_EQ(i, 0);

  MooseUtils::SemidynamicVector<Custom, 100, true> test2(100);
  for (auto & i : test2)
    EXPECT_EQ(i._data, 333);
}

TEST(MooseUtils, isZero)
{
  EXPECT_TRUE(MooseUtils::isZero(Real(0)));
  EXPECT_TRUE(MooseUtils::isZero(ADReal(0)));
  EXPECT_TRUE(MooseUtils::isZero(RealVectorValue(0)));
  EXPECT_TRUE(MooseUtils::isZero(ADRealVectorValue(0)));
  EXPECT_TRUE(MooseUtils::isZero(RealTensorValue(0)));
  EXPECT_TRUE(MooseUtils::isZero(ADRealTensorValue(0)));
  EXPECT_TRUE(MooseUtils::isZero(std::vector<Real>(1, 0)));
  EXPECT_TRUE(MooseUtils::isZero(std::array<Real, 1>{{0}}));

  EXPECT_TRUE(!MooseUtils::isZero(Real(1)));
  EXPECT_TRUE(!MooseUtils::isZero(ADReal(1)));
  EXPECT_TRUE(!MooseUtils::isZero(RealVectorValue(1)));
  EXPECT_TRUE(!MooseUtils::isZero(ADRealVectorValue(1)));
  EXPECT_TRUE(!MooseUtils::isZero(RealTensorValue(1)));
  EXPECT_TRUE(!MooseUtils::isZero(ADRealTensorValue(1)));
  EXPECT_TRUE(!MooseUtils::isZero(std::vector<Real>(1, 1)));
  EXPECT_TRUE(!MooseUtils::isZero(std::array<Real, 1>{{1}}));
}
