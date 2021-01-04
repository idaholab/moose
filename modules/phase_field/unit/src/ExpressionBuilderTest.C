//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ExpressionBuilder.h"

class ExpressionBuilderTest : public ::testing::Test, public ExpressionBuilder
{
};

TEST_F(ExpressionBuilderTest, test)
{
  EBTerm x;
  x = "x";

  EBTerm y("y");
  EBTerm z("z"), u("u"), v("v"), w("w");

  EBTerm a(1), b(4), c(8.9);

  EBTerm f = pow(x, 2) + pow(y, a) + 3 * y * (-a);

  EBFunction G, H;

  // Test using functions in terms
  {
    G((x, y)) = f;
    H((x)) = -G((x, b)) + c % b;

    EXPECT_EQ(std::string(H), "-(x^2+4^1+3*4*-1)+8.9%4");
    EXPECT_EQ(G.args(), "x,y");
  }

  // Test substitution subtleties (this would return x^x if substitution were performed
  // sequentially, which they are not.. ..because that would be dumb!)
  {
    G((x, y)) = pow(x, y);
    H((x, y, z)) = x * y * z;
    EXPECT_EQ(std::string(G), "x^y");
    EXPECT_EQ(std::string(G((y, x))), "y^x");
    EXPECT_EQ(std::string(H((z, y, x))), "z*y*x");
  }

  // Test single bracket syntax
  {
    G(x, y, z) = x * y * z;
    H(u, v, w, x, y, z) = u + v + w + x + y + z;

    EXPECT_EQ(std::string(G(a, b, c)), "1*4*8.9");
    EXPECT_EQ(std::string(H(a, b, c, a, b, c)), "1+4+8.9+1+4+8.9");
    EXPECT_EQ(std::string(G(a, b, c) + H(a, b, c, a, b, c)), "1*4*8.9+1+4+8.9+1+4+8.9");

    H(u, v, w, x, y, z) = u + v + w + G(y, x, x);
    EXPECT_EQ(std::string(H(a, b, c, x, y, z)), "1+4+8.9+y*x*x");
  }

  // Test associativity
  {
    EBTerm def = x - y - z;
    EBTerm left = (x - y) - z;
    EBTerm right = x - (y - z);

    EXPECT_EQ(std::string(def), "x-y-z");
    EXPECT_EQ(std::string(left), "x-y-z");
    EXPECT_EQ(std::string(right), "x-(y-z)");
  }

  // test comparison operators
  {
    EBTerm comp1 = (x < y) + (x > y);
    EBTerm comp2 = (x <= y) + (x >= y);
    EBTerm comp3 = (x == y) + (x != y);

    EXPECT_EQ(std::string(comp1), "(x<y)+(x>y)");
    EXPECT_EQ(std::string(comp2), "(x<=y)+(x>=y)");
    EXPECT_EQ(std::string(comp3), "(x=y)+(x!=y)");
  }

  // test binary functions
  {
    EBTerm comp4 = atan2(x, y);
    EXPECT_EQ(std::string(comp4), "atan2(x,y)");
  }

  // test ifexpr
  {
    EBTerm if1 = conditional(x < 2 * y, x * x + y, y * y + x);
    EXPECT_EQ(std::string(if1),
              "if"
              "(x<2*y,x*x+y,y*y+x)");
  }

  // test temp id node stringify
  {
    EBTerm temp1;
    EBTerm temp2;
    EXPECT_NE(std::string(temp1), std::string(temp2));
  }

  // test substitution
  {
    // plog substitution
    EBTerm u = log(x / a);
    u.substitute(EBLogPlogSubstitution(b));
    EXPECT_EQ(std::string(u), "plog(x/1,4)");

    // single substitution in a ternary
    EBTerm v = conditional(x < y, x * y, x / y);
    v.substitute(EBTermSubstitution(x, a));
    EXPECT_EQ(std::string(v),
              "if"
              "(1<y,1*y,1/y)");

    // substitution list
    EBTerm w = conditional(x < y, x * y, x / y);
    EBSubstitutionRuleList s(2);
    EBTermSubstitution s0(x, y), s1(y, a);
    s[0] = &s0;
    s[1] = &s1;
    w.substitute(s);
    EXPECT_EQ(std::string(w),
              "if"
              "(y<1,y*1,y/1)");
  }
}
