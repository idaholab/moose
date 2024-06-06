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
    EXPECT_NEAR(r1, r2, 1e-13);                                                                    \
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

  CTD_EVALTEST(x * -1.0 > x * -2.0, -10, 10, 0.63)
  CTD_EVALTEST(x * -1.0 < x * -2.0, -10, 10, 0.63)
  CTD_EVALTEST(x * -1.0 >= x * -2.0, -10, 10, 1)
  CTD_EVALTEST(x * -1.0 <= x * -2.0, -10, 10, 1)
  CTD_EVALTEST(0.0 < x, -1, 1, 1)
  CTD_EVALTEST(0.0 > x, -1, 1, 1)
  CTD_EVALTEST(0.0 <= x, -1, 1, 1)
  CTD_EVALTEST(0.0 >= x, -1, 1, 1)
  CTD_EVALTEST(0.0 == x, -1, 1, 1)
  CTD_EVALTEST(0.0 != x, -1, 1, 1)

  using namespace std;
  CTD_EVALTEST(sin(x), -10, 10, 0.72)
  CTD_EVALTEST(cos(x), -10, 10, 0.72)
  CTD_EVALTEST(tan(x), -10, 10, 0.72)
  CTD_EVALTEST(exp(x), -2, 2, 0.1)
  CTD_EVALTEST(erf(x), -2, 2, 0.1)
  CTD_EVALTEST(log(x), 0.1, 10, 0.1)
  CTD_EVALTEST(tanh(x), -10, 10, 0.1)
  CTD_EVALTEST(sinh(x), -4, 4, 0.1)
  CTD_EVALTEST(cosh(x), -4, 4, 0.1)
  CTD_EVALTEST(atan(x), -4, 4, 0.1)

  CTD_EVALTEST(min(x, x * x), -2, 2, 0.271)
  CTD_EVALTEST(max(x, x * x), -2, 2, 0.271)

  const auto v = makeValue(0.5);
  const auto r1 = CompileTimeDerivatives::atan2(1.0, 2.0);
  const auto r2 = CompileTimeDerivatives::atan2(v, 2.0);
  const auto r3 = CompileTimeDerivatives::atan2(1.0, v);
  const auto r4 = CompileTimeDerivatives::atan2(v, v);
  EXPECT_NEAR(r1(), 0.46364760900080609, 1e-12);
  EXPECT_NEAR(r2(), 0.24497866312686414, 1e-12);
  EXPECT_NEAR(r3(), 1.1071487177940904, 1e-12);
  EXPECT_NEAR(r4(), 0.7853981633974482, 1e-12);
}

TEST(CompileTimeDerivativesTest, finitedifference)
{
  Real v = 0;
  const auto x = makeRef<1>(v);

  const auto test = [&v](auto expr, Real v0, Real v1, Real dv, Real eps = 1e-6, Real err = 1e-6)
  {
    for (Real vv = v0; vv <= v1; vv += dv)
    {
      v = vv;
      const auto df = expr.template D<1>()();
      v = vv - eps;
      const auto f0 = expr();
      v = vv + eps;
      const auto f1 = expr();
      const auto fd = (f1 - f0) / (2.0 * eps);
      EXPECT_NEAR(df, fd, err);
    }
  };

  test(x, -3, 3, 0.21);
  test(-x, -3, 3, 0.21);
  test(x * x, -3, 3, 0.21);
  test(pow(x, 3), -3, 3, 0.21);
  test(pow(5, x), -3, 3, 0.21);
  test(pow(x, 3.0), -3, 3, 0.21);
  test(pow(5.0, x), -3, 3, 0.21);
  test(pow(x, x), 0.1, 3, 0.21);
  test(pow<4>(x), -3, 3, 0.21);
  test(pow<4>(2), -1, 1, 0.4);
  test(sin(x), -3, 3, 0.21);
  test(-sin(x), -3, 3, 0.21);
  test(cos(x), -3, 3, 0.21);
  test(tan(x), -10, 10, 0.2, 1e-7);
  test(exp(x), -2, 2, 0.2);
  test(erf(x), -2, 2, 0.2);
  test(log(x), 0.1, 3, 0.1);
  test(tanh(x), -10, 10, 0.2, 1e-7);
  test(sinh(x), -4, 4, 0.2);
  test(cosh(x), -4, 4, 0.2);
  test(atan(x), -4, 4, 0.2);
  test(atan2(x, 1), -3, 3, 0.21);
  test(atan2(1, x), -3, 3, 0.21);
  test(atan2(sin(x), cos(x)), -3, 3, 0.2);
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

TEST(CompileTimeDerivativesTest, print)
{
  std::vector<double> _prop{1, 2, 3, 4};
  std::size_t _qp = 2;
  const auto prop = makeRef(_prop, _qp);
  const auto result1 = 3.0 * prop * prop;
  EXPECT_EQ(result1.print(), "3*[a[2]]*[a[2]]");

  Real v = 0.0;
  const auto x = makeRef(v);
  const auto result2 = x * (1.0 - x) - (x * log(x) + (1.0 - x) * log(1.0 - x));
  EXPECT_EQ(result2.print(), "[v]*(1-[v])-([v]*log([v])+(1-[v])*log(1-[v]))");
}

TEST(CompileTimeDerivativesTest, makeRefs)
{
  const Real va = 1, vb = 2, vc = 1.5;
  const auto [a, b, c] = makeRefs<30>(va, vb, vc);

  // matching order
  EXPECT_EQ(&va, &a());
  EXPECT_EQ(&vb, &b());
  EXPECT_EQ(&vc, &c());

  // correct tags
  EXPECT_EQ(a.D<30>()(), 1);
  EXPECT_EQ(a.D<31>()(), 0);
  EXPECT_EQ(a.D<32>()(), 0);

  EXPECT_EQ(b.D<30>()(), 0);
  EXPECT_EQ(b.D<31>()(), 1);
  EXPECT_EQ(b.D<32>()(), 0);

  EXPECT_EQ(c.D<30>()(), 0);
  EXPECT_EQ(c.D<31>()(), 0);
  EXPECT_EQ(c.D<32>()(), 1);
}

TEST(CompileTimeDerivativesTest, makeStandardDeviation)
{
  // start tag for the fitting parameters
  const int params = 30;

  // fitting parameter data and corresponding CTD objects
  const Real va = 1, vb = 2, vc = 1.5;
  const auto [a, b, c] = makeRefs<params>(va, vb, vc);

  // function variable (omit tag, since we dont need to derive w.r.t. x)
  const Real vx = 0.5;
  const auto x = makeRef(vx);

  // function expression
  const auto f = a + b * x + c * x * x;

  // covariance matrix for the a,b,c parameters
  // clang-format off
  CTMatrix<Real, 3, 3> covariance(
    0.2,  0.01, 0.07,
    0.01, 0.4,  0.05,
    0.07, 0.05, 0.3);
  // clang-format on

  // Object that calculates the standard deviation of f
  const auto std_dev = makeStandardDeviation<params>(f, covariance);

  EXPECT_NEAR(std_dev(), 0.6133922073192649, 1e-15);
}

TEST(CompileTimeDerivativesTest, conditional)
{
  Real vx = 0.0;
  const auto x = makeRef(vx);

  const auto result = conditional(x < 3, 2 * x, 5 * x);

  for (vx = 0.0; vx < 6.0; vx += 0.31)
  {
    if (vx < 3)
      EXPECT_EQ(result(), 2 * vx);
    else
      EXPECT_EQ(result(), 5 * vx);
  }
}
