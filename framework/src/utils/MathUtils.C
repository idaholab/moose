//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MathUtils.h"
#include "libmesh/utility.h"

namespace MathUtils
{

Real
plainLog(Real x, unsigned int derivative_order)
{
  switch (derivative_order)
  {
    case 0:
      return std::log(x);

    case 1:
      return 1.0 / x;

    case 2:
      return -1.0 / (x * x);

    case 3:
      return 2.0 / (x * x * x);
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

Real
poly1Log(Real x, Real tol, unsigned int derivative_order)
{
  if (x >= tol)
    return plainLog(x, derivative_order);

  const auto c1 = [&]() { return 1.0 / tol; };
  const auto c2 = [&]() { return std::log(tol) - 1.0; };

  switch (derivative_order)
  {
    case 0:
      return c1() * x + c2();

    case 1:
      return c1();

    case 2:
      return 0.0;

    case 3:
      return 0.0;
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

Real
poly2Log(Real x, Real tol, unsigned int derivative_order)
{
  if (x >= tol)
    return plainLog(x, derivative_order);

  const auto c1 = [&]() { return -0.5 / (tol * tol); };
  const auto c2 = [&]() { return 2.0 / tol; };
  const auto c3 = [&]() { return std::log(tol) - 3.0 / 2.0; };

  switch (derivative_order)
  {
    case 0:
      return c1() * x * x + c2() * x + c3();

    case 1:
      return 2.0 * c1() * x + c2();

    case 2:
      return 2.0 * c1();

    case 3:
      return 0.0;
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

Real
poly3Log(Real x, Real tol, unsigned int derivative_order)
{
  if (x >= tol)
    return plainLog(x, derivative_order);

  const auto c1 = [&]() { return 1.0 / (3.0 * tol * tol * tol); };
  const auto c2 = [&]() { return -3.0 / (2.0 * tol * tol); };
  const auto c3 = [&]() { return 3.0 / tol; };
  const auto c4 = [&]() { return std::log(tol) - 11.0 / 6.0; };

  switch (derivative_order)
  {
    case 0:
      return c1() * x * x * x + c2() * x * x + c3() * x + c4();

    case 1:
      return 3.0 * c1() * x * x + 2.0 * c2() * x + c3();

    case 2:
      return 6.0 * c1() * x + 2.0 * c2();

    case 3:
      return 6.0 * c1();
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

Real
poly4Log(Real x, Real tol, unsigned int derivative_order)
{
  if (x >= tol)
    return plainLog(x, derivative_order);

  switch (derivative_order)
  {
    case 0:
      return std::log(tol) + (x - tol) / tol -
             Utility::pow<2>(x - tol) / (2.0 * Utility::pow<2>(tol)) +
             Utility::pow<3>(x - tol) / (3.0 * Utility::pow<3>(tol)) -
             Utility::pow<4>(x - tol) / (4.0 * Utility::pow<4>(tol)) +
             Utility::pow<5>(x - tol) / (5.0 * Utility::pow<5>(tol)) -
             Utility::pow<6>(x - tol) / (6.0 * Utility::pow<6>(tol));

    case 1:
      return 1.0 / tol - (x - tol) / Utility::pow<2>(tol) +
             Utility::pow<2>(x - tol) / Utility::pow<3>(tol) -
             Utility::pow<3>(x - tol) / Utility::pow<4>(tol) +
             Utility::pow<4>(x - tol) / Utility::pow<5>(tol) -
             Utility::pow<5>(x - tol) / Utility::pow<6>(tol);

    case 2:
      return -1.0 / Utility::pow<2>(tol) + 2.0 * (x - tol) / Utility::pow<3>(tol) -
             3.0 * Utility::pow<2>(x - tol) / Utility::pow<4>(tol) +
             4.0 * Utility::pow<3>(x - tol) / Utility::pow<5>(tol) -
             5.0 * Utility::pow<4>(x - tol) / Utility::pow<6>(tol);

    case 3:
      return 2.0 / Utility::pow<3>(tol) - 6.0 * (x - tol) / Utility::pow<4>(tol) +
             12.0 * Utility::pow<2>(x - tol) / Utility::pow<5>(tol) -
             20.0 * Utility::pow<3>(x - tol) / Utility::pow<6>(tol);
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

/// \todo This can be done without std::pow!
Real
taylorLog(Real x)
{
  Real y = (x - 1.0) / (x + 1.0);
  Real val = 1.0;
  for (unsigned int i = 0; i < 5; ++i)
  {
    Real exponent = i + 2.0;
    val += 1.0 / (2.0 * (i + 1.0) + 1.0) * std::pow(y, exponent);
  }

  return val * 2.0 * y;
}

} // namespace MathUtils
