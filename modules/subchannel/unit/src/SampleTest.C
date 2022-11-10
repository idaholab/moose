/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "gtest/gtest.h"

TEST(MySampleTests, descriptiveTestName)
{
  // compare equality
  EXPECT_EQ(2, 1 + 1);
  EXPECT_DOUBLE_EQ(2 * 3.5, 1.0 * 8 - 1);

  // compare equality and immediately terminate this test if it fails
  // ASSERT_EQ(2, 1);

  // this won't run if you uncomment the above test because above assert will fail
  ASSERT_NO_THROW(1 + 1);

  // for a complete list of assertions and for more unit testing documentation see:
  // https://github.com/google/googletest/blob/master/googletest/docs/Primer.md
}

TEST(MySampleTests, anotherTest)
{
  EXPECT_LE(1, 2);
  // ...
}
