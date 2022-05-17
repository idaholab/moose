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
#include "MooseTypes.h"
#include "CompileTimeDerivatives.h"

using namespace CompileTimeDerivatives;

TEST(CompileTimeDerivativesTest, simple)
{
  // this test serves as a simple example for the compile time derivative system

  // a variable to be used in the expression evaluation
  Real my_x = 1.0;

  // use an enum to use symbols for tags
  enum
  {
    dx
  };

  // we wrap this variable in a reference node that we tag with the `dx` enum
  const auto x = makeRef<dx>(my_x);

  // now we build an expression that uses our variable x
  // the expression is not yet evaluated
  const auto result = (x + 2) * (x + 3);

  // when evaluated this expression should always return (1+2)*(1*3) = 12
  // we evaluate the result using the () operator
  EXPECT_NEAR(result(), 12, 1e-10);

  // we can build the derivative using the D<>() function. The template parameter
  // is the tag we want to take the derivative w.r.t.
  const auto dx_result = result.D<dx>();

  // the derivative of the above expression is 1*(x+3)+(x+2)*1 = 2x+5
  // let's set our original variable to 2
  my_x = 2.0;

  // if we re-evaluate the original expression we should get an updated result
  // of (2+2)*(2*3) = 20
  EXPECT_NEAR(result(), 20, 1e-10);

  // and for the derivative we expect 2*2+5 = 9
  EXPECT_NEAR(dx_result(), 9, 1e-10);
}

#define CTD_EVALTEST(expression, v0, v1, dv)                                                       \
  for (Real v = v0; v <= v1; v += dv)                                                              \
  {                                                                                                \
    Real r1, r2;                                                                                   \
    {                                                                                              \
      const auto x = makeRef<1>(v);                                                                \
      const auto result = expression;                                                              \
      r1 = result();                                                                               \
    }                                                                                              \
    {                                                                                              \
      const auto & x = v;                                                                          \
      r2 = expression;                                                                             \
    }                                                                                              \
    EXPECT_EQ(r1, r2);                                                                             \
  }

TEST(CompileTimeDerivativesTest, evaluate)
{
  CTD_EVALTEST(x, -10, 10, 0.72)
  CTD_EVALTEST(2 * x, -10, 10, 0.72)
  CTD_EVALTEST(2.0 * x, -10, 10, 0.72)
  CTD_EVALTEST(x * 3, -10, 10, 0.72)
  CTD_EVALTEST(x * -3.0, -10, 10, 0.72)
  CTD_EVALTEST(x + 1, -10, 10, 0.72)
  CTD_EVALTEST(x - 1.0, -10, 10, 0.72)
  CTD_EVALTEST(0.5 + x, -10, 10, 0.72)
  CTD_EVALTEST(0.75 - x, -10, 10, 0.72)
  CTD_EVALTEST(1.0 / x, -10, 10, 0.72)
  CTD_EVALTEST(x / 2.0, -10, 10, 0.72)
  CTD_EVALTEST(x + (2.0 / x), -10, 10, 0.72)
  CTD_EVALTEST(x * (1.0 + x), -10, 10, 0.72)

  CTD_EVALTEST(x * (1.0 + x * (3.0 - x * (2.0 + x * (5.0 - x)))), -10, 10, 0.63)

  using namespace std;
  CTD_EVALTEST(sin(x), -10, 10, 0.72)
  CTD_EVALTEST(cos(x), -10, 10, 0.72)
  CTD_EVALTEST(tan(x), -10, 10, 0.72)
  CTD_EVALTEST(exp(x), -2, 2, 0.1)
  CTD_EVALTEST(log(x), 0.1, 10, 0.1)
}

TEST(CompileTimeDerivativesTest, derivative)
{
  enum
  {
    dX
  };

  Real v = 0.0;
  const auto x = makeRef<dX>(v);
  const auto result = x * (1.0 - x) - (x * log(x) + (1.0 - x) * log(1.0 - x));

  Real r0 = 0, r1 = 0, r2 = 0;
  for (v = 0.01; v <= 0.99; v += 0.01)
  {
    r0 += std::abs(result() - (v * (1.0 - v) - (v * std::log(v) + (1.0 - v) * std::log(1.0 - v))));
    r1 += std::abs(result.D<dX>()() -
                   (-2.0 * v - std::log(v) + std::log(1.0 - v) - (v - 1.0) / (1.0 - v)));
    r2 += std::abs(result.D<dX>().D<dX>()() - (-2.0 + 1.0 / (v - 1.0) - 1.0 / v));
  }
  EXPECT_NEAR(r0, 0, 1e-12);
  EXPECT_NEAR(r1, 0, 1e-12);
  EXPECT_NEAR(r2, 0, 1e-12);
}

TEST(CompileTimeDerivativesTest, variable_reference)
{
  enum
  {
    dX
  };

  Real x = 0.0;
  const auto X = makeRef<dX>(x);
  const auto result = X * X + 100.0;

  x = 5;
  EXPECT_EQ(result(), 125.0);
  EXPECT_EQ(result.D<dX>()(), 10.0);

  x = 3;
  EXPECT_EQ(result(), 109.0);
  EXPECT_EQ(result.D<dX>()(), 6.0);
}

TEST(CompileTimeDerivativesTest, vector_reference)
{
  enum
  {
    dX
  };

  std::vector<double> _prop{1, 2, 3, 4};
  std::size_t _qp = 0;

  const auto prop = makeRef<dX>(_prop, _qp);
  const auto result = 3.0 * prop * prop;

  _qp = 1;
  EXPECT_EQ(result(), 12.0);
  EXPECT_EQ(result.D<dX>()(), 12.0);
  EXPECT_EQ(result.D<dX>().D<dX>()(), 6.0);

  _qp = 3;
  EXPECT_EQ(result(), 48.0);
  EXPECT_EQ(result.D<dX>()(), 24.0);
  EXPECT_EQ(result.D<dX>().D<dX>()(), 6.0);
}
