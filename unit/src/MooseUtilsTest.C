/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

template <typename T>
void
stringToNumberHelper(const std::vector<std::string> & inputs)
{
  for (const std::string & str : inputs)
  {
    try
    {
      MooseUtils::stringToNumber<T>(str);
    }
    catch (const std::exception & e)
    {
      T item;
      std::string msg(e.what());
      std::string gold =
          "Failed to convert '" + str + "' to the supplied type of " + typeid(item).name() + ".";
      ASSERT_NE(msg.find(gold), std::string::npos) << "failed with unexpected error: " << msg;
    }
  }
}

TEST(MooseUtils, stringToNumber)
{
  {
    short int gold = 42;
    EXPECT_EQ(MooseUtils::stringToNumber<short int>("42"), gold);

    gold = -42;
    EXPECT_EQ(MooseUtils::stringToNumber<short int>("-42"), gold);
  }

  {
    unsigned short int gold = 42;
    EXPECT_EQ(MooseUtils::stringToNumber<unsigned short int>("42"), gold);
  }

  {
    int gold = 42;
    EXPECT_EQ(MooseUtils::stringToNumber<int>("42"), gold);

    gold = -42;
    EXPECT_EQ(MooseUtils::stringToNumber<int>("-42"), gold);
  }

  {
    unsigned int gold = 42;
    EXPECT_EQ(MooseUtils::stringToNumber<unsigned int>("42"), gold);
  }

  {
    long int gold = 42;
    EXPECT_EQ(MooseUtils::stringToNumber<long int>("42"), gold);

    gold = -42;
    EXPECT_EQ(MooseUtils::stringToNumber<long int>("-42"), gold);
  }

  {
    unsigned long int gold = 42;
    EXPECT_EQ(MooseUtils::stringToNumber<unsigned long int>("42"), gold);
  }

  {
    long long int gold = 42;
    EXPECT_EQ(MooseUtils::stringToNumber<long long int>("42"), gold);

    gold = -42;
    EXPECT_EQ(MooseUtils::stringToNumber<long long int>("-42"), gold);
  }

  {
    unsigned long long int gold = 42;
    EXPECT_EQ(MooseUtils::stringToNumber<unsigned long long int>("42"), gold);
  }

  {
    float gold = 1.2;
    EXPECT_EQ(MooseUtils::stringToNumber<float>("1.2"), gold);

    gold = -1.2;
    EXPECT_EQ(MooseUtils::stringToNumber<float>("-1.2"), gold);

    gold = 1.2e5;
    EXPECT_EQ(MooseUtils::stringToNumber<float>("1.2e5"), gold);

    gold = -1.2e-5;
    EXPECT_EQ(MooseUtils::stringToNumber<float>("-1.2e-5"), gold);
  }

  {
    double gold = 1.2;
    EXPECT_EQ(MooseUtils::stringToNumber<double>("1.2"), gold);

    gold = -1.2;
    EXPECT_EQ(MooseUtils::stringToNumber<double>("-1.2"), gold);
  }

  {
    long double gold = 1.2L;
    EXPECT_EQ(MooseUtils::stringToNumber<long double>("1.2"), gold);

    gold = -1.2L;
    EXPECT_EQ(MooseUtils::stringToNumber<long double>("-1.2"), gold);
  }

  std::vector<std::string> failures = {"", "42 ", "42foo", "42.1"};
  stringToNumberHelper<short int>(failures);
  stringToNumberHelper<int>(failures);
  stringToNumberHelper<long int>(failures);
  stringToNumberHelper<long long int>(failures);

  failures = {"", "42 ", "42 ", "42.1", "-42"};
  stringToNumberHelper<unsigned short int>(failures);
  stringToNumberHelper<unsigned int>(failures);
  stringToNumberHelper<unsigned long int>(failures);
  stringToNumberHelper<unsigned long long int>(failures);

  failures = {"", "42.1 ", "42.1foo"};
  stringToNumberHelper<float>(failures);
  stringToNumberHelper<double>(failures);
  stringToNumberHelper<long double>(failures);
}
