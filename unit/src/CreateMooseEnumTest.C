//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseEnum.h"

TEST(CreateMooseEnumTest, defaultValues)
{
  struct C
  {
    CreateMooseEnumClass(DefaultValue, ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN);
    CreateMooseEnumClass(ExplicitDefaultValue, ZERO = 0, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN);
  };
  EXPECT_EQ(C::getDefaultValueOptions(), "ZERO ONE TWO THREE FOUR FIVE SIX SEVEN");
  EXPECT_EQ(C::getExplicitDefaultValueOptions(), "ZERO=0 ONE TWO THREE FOUR FIVE SIX SEVEN");
}

TEST(CreateMooseEnumTest, explicitIntegers)
{
  struct C
  {
    CreateMooseEnumClass(Offset, TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN);
    CreateMooseEnumClass(Bounce, TWO = 2, FOUR = 4, FIVE, THREE = 3, SIX = 6, SEVEN);
    CreateMooseEnumClass(Skip, ONE, TWO, THREE, FIVE = 5, SIX, SEVEN);
  };
  EXPECT_EQ(C::getOffsetOptions(), "TWO=2 THREE FOUR FIVE SIX SEVEN");
  EXPECT_EQ(C::getBounceOptions(), "TWO=2 FOUR=4 FIVE THREE=3 SIX=6 SEVEN");
  EXPECT_EQ(C::getSkipOptions(), "ONE TWO THREE FIVE=5 SIX SEVEN");

  EXPECT_EQ(static_cast<int>(C::Offset::FIVE), 5);
  EXPECT_EQ(static_cast<int>(C::Bounce::THREE), 3);
  EXPECT_EQ(static_cast<int>(C::Skip::SEVEN), 7);
}

TEST(CreateMooseEnumTest, member)
{
  struct C
  {
    CreateMooseEnumClass(Number, THREE = 3, FOUR, FIVE, SIX, SEVEN) number = static_cast<Number>(6);
  } c;
  EXPECT_EQ(c.number, C::Number::SIX);
}
