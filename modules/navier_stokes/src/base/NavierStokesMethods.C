//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesMethods.h"
#include "MooseError.h"
#include "libmesh/vector_value.h"

namespace NS
{
int
delta(unsigned int i, unsigned int j)
{
  if (i == j)
    return 1;
  else
    return 0;
}

int
computeSign(const Real & a)
{
  return a > 0 ? 1 : (a < 0 ? -1 : 0);
}

unsigned int
getIndex(const Real & p, const std::vector<Real> & bounds)
{
  if (p < bounds.front() || p > bounds.back())
    mooseError("Point exceeds bounds of domain!");

  for (unsigned int i = 1; i < bounds.size(); ++i)
    if (p <= bounds[i])
      return i - 1;

  return bounds.size() - 2;
}

Real
reynoldsPropertyDerivative(
    const Real & Re, const Real & rho, const Real & mu, const Real & drho, const Real & dmu)
{
  return Re * (drho / std::max(rho, 1e-6) - dmu / std::max(mu, 1e-8));
}

Real
prandtlPropertyDerivative(const Real & mu,
                          const Real & cp,
                          const Real & k,
                          const Real & dmu,
                          const Real & dcp,
                          const Real & dk)
{
  return (k * (mu * dcp + cp * dmu) - mu * cp * dk) / std::max(k * k, 1e-8);
}

ADReal
findUStar(const ADReal & mu, const ADReal & rho, const ADReal & u, const Real dist)
{
  // usually takes about 3-4 iterations
  constexpr int MAX_ITERS{50};
  constexpr Real REL_TOLERANCE{1e-6};

  constexpr Real von_karman{0.4187};

  const ADReal nu = mu / rho;

  ADReal u_star = std::sqrt(nu * u / dist);

  // Newton-Raphson method to solve for u_star (friction velocity).
  for (int i = 0; i < MAX_ITERS; ++i)
  {
    ADReal residual = u_star / von_karman * std::log(u_star * dist / (0.111 * nu)) - u;
    ADReal deriv = (1 + std::log(u_star * dist / (0.111 * nu))) / von_karman;
    ADReal new_u_star = std::max(1e-20, u_star - residual / deriv);

    Real rel_err = std::abs((new_u_star.value() - u_star.value()) / new_u_star.value());

    u_star = new_u_star;
    if (rel_err < REL_TOLERANCE)
      return u_star;
  }

  mooseException("Could not find the wall friction velocity (mu: ",
                 mu,
                 " rho: ",
                 rho,
                 " velocity: ",
                 u,
                 " wall distance: ",
                 dist,
                 ")");
}

ADReal
computeSpeed(const ADRealVectorValue & velocity)
{
  // if the velocity is zero, then the norm function call fails because AD tries to calculate the
  // derivatives which causes a divide by zero - because d/dx(sqrt(f(x))) = 1/2/sqrt(f(x))*df/dx.
  // So add a bit of noise (based on hitchhiker's guide to the galaxy's meaning of life number) to
  // avoid this failure mode.
  return isZero(velocity) ? 1e-42 : velocity.norm();
}
}
