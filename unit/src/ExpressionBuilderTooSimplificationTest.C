//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ExpressionBuilderToo.h"

class ExpressionBuilderTooSimplificationTest : public ::testing::Test, public ExpressionBuilderToo
{
};

TEST_F(ExpressionBuilderTooSimplificationTest, test)
{
  EBTerm a("a");

  EBTerm b = a + a;

  b.simplify();
  EXPECT_EQ(std::string(b), "a*2");

  b = a + a - a - a + a - a + a + a + a + a - a + 2 * a;
  b.simplify();
  EXPECT_EQ(std::string(b), "a*5");

  b = pow(a, 3) + 4 * pow(a, 3) + 16 * a + 4 * a + 25;
  b.simplify();
  EXPECT_EQ(std::string(b), "a^3*5+a*20+25");

  EBTerm c("c");

  b = cos(sin(cos(a + c))) + cos(sin(cos(c + a)));
  b.simplify();
  EXPECT_EQ(std::string(b), "cos(sin(cos(a+c)))*2");

  EBTerm d("d");

  b = pow(a, 3) * pow(c, 3) * pow(d, 3) / (pow(a, 7) * pow(c, 2) * pow(d, 3));
  b.simplify();
  EXPECT_EQ(std::string(b), "c*a^-4");

  b = pow(cos(a), 2) + pow(sin(a), 2);
  b.simplify();
  EXPECT_EQ(std::string(b), "1");

  b = pow(cos(a), 2) + a + a - a - a + a - a + a + a + a + a - a + 2 * a + pow(sin(a), 2);
  b.simplify();
  EXPECT_EQ(std::string(b), "a*5+1");

  b = a * pow(a, 2);
  b.simplify();
  EXPECT_EQ(std::string(b), "a^3");

  b = 4 * (a + 3 * c) - (7 + 4 * c) + 2 * a;
  b.simplify();
  EXPECT_EQ(std::string(b), "c*8+a*6+-7");

  b = ((3 - a) * (a + 2) + (-a + 4) * (7 * a + 2) - (a - c) * (2 * a - c)) - 3 * a;
  b.simplify();
  EXPECT_EQ(std::string(b), "c^2*-1+a*(c+(c+-1)*2+26)+a^2*-10+14");

  b = pow(pow(a, 3) * pow(c, 3) * pow(d, 3), 3) / (pow(a, 7) * pow(c, 2) * pow(d, 3));
  b.simplify();
  EXPECT_EQ(std::string(b), "c^7*a^2*d^6");

  b = ((a == a) == c);
  b.simplify();
  std::cout << std::string(b) << std::endl;

  b = (pow(4, a) + pow(2, 2 * a)) / pow(2, a);
  b.simplify();

  b = (a == a);
  b.simplify();
  EXPECT_EQ(std::string(b), "1");

  std::cout << std::string(b) << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
}
