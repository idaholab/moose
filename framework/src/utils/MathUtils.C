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
plainLog(Real x, int deriv)
{
  switch (deriv)
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

  mooseError("Unsupported derivative order ", deriv);
}

Real
poly1Log(Real x, Real tol, int deriv)
{
  if (x >= tol)
    return plainLog(x, deriv);

  const auto c1 = [&]() { return 1.0 / tol; };
  const auto c2 = [&]() { return std::log(tol) - 1.0; };

  switch (deriv)
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

  mooseError("Unsupported derivative order ", deriv);
}

Real
poly2Log(Real x, Real tol, int deriv)
{
  if (x >= tol)
    return plainLog(x, deriv);

  const auto c1 = [&]() { return -0.5 / (tol * tol); };
  const auto c2 = [&]() { return 2.0 / tol; };
  const auto c3 = [&]() { return std::log(tol) - 3.0 / 2.0; };

  switch (deriv)
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

  mooseError("Unsupported derivative order ", deriv);
}

Real
poly3Log(Real x, Real tol, int deriv)
{
  if (x >= tol)
    return plainLog(x, deriv);

  const auto c1 = [&]() { return 1.0 / (3.0 * tol * tol * tol); };
  const auto c2 = [&]() { return -3.0 / (2.0 * tol * tol); };
  const auto c3 = [&]() { return 3.0 / tol; };
  const auto c4 = [&]() { return std::log(tol) - 11.0 / 6.0; };

  switch (deriv)
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

  mooseError("Unsupported derivative order ", deriv);
}

Real
poly4Log(Real x, Real tol, int deriv)
{
  if (x >= tol)
    return plainLog(x, deriv);

  switch (deriv)
  {
    case 0:
      return std::log(tol) + (x - tol) / tol - (x - tol) * (x - tol) / (2.0 * tol * tol) +
             (x - tol) * (x - tol) * (x - tol) / (3.0 * tol * tol * tol) -
             (x - tol) * (x - tol) * (x - tol) * (x - tol) / (4.0 * tol * tol * tol * tol) +
             (x - tol) * (x - tol) * (x - tol) * (x - tol) * (x - tol) /
                 (5.0 * tol * tol * tol * tol * tol) -
             (x - tol) * (x - tol) * (x - tol) * (x - tol) * (x - tol) * (x - tol) /
                 (6.0 * tol * tol * tol * tol * tol * tol);

    case 1:
      return 1.0 / tol - 2.0 * (x - tol) / (2.0 * tol * tol) +
             3.0 * (x - tol) * (x - tol) / (3.0 * tol * tol * tol) -
             4.0 * (x - tol) * (x - tol) * (x - tol) / (4.0 * tol * tol * tol * tol) +
             5.0 * (x - tol) * (x - tol) * (x - tol) * (x - tol) /
                 (5.0 * tol * tol * tol * tol * tol) -
             6.0 * (x - tol) * (x - tol) * (x - tol) * (x - tol) * (x - tol) /
                 (6.0 * tol * tol * tol * tol * tol * tol);

    case 2:
      return -2.0 * 1.0 / (2.0 * tol * tol) + 3.0 * 2.0 * (x - tol) / (3.0 * tol * tol * tol) -
             4.0 * 3.0 * (x - tol) * (x - tol) / (4.0 * tol * tol * tol * tol) +
             5.0 * 4.0 * (x - tol) * (x - tol) * (x - tol) / (5.0 * tol * tol * tol * tol * tol) -
             6.0 * 5.0 * (x - tol) * (x - tol) * (x - tol) * (x - tol) /
                 (6.0 * tol * tol * tol * tol * tol * tol);

    case 3:
      return 3.0 * 2.0 * 1.0 / (3.0 * tol * tol * tol) -
             4.0 * 3.0 * 2.0 * (x - tol) / (4.0 * tol * tol * tol * tol) +
             5.0 * 4.0 * 3.0 * (x - tol) * (x - tol) / (5.0 * tol * tol * tol * tol * tol) -
             6.0 * 5.0 * 4.0 * (x - tol) * (x - tol) * (x - tol) /
                 (6.0 * tol * tol * tol * tol * tol * tol);
  }

  mooseError("Unsupported derivative order ", deriv);
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
