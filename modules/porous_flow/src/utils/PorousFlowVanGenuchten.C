/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowVanGenuchten.h"
#include "libmesh/utility.h"

namespace PorousFlowVanGenuchten
{
Real
seff(Real p, Real al, Real m)
{
  Real n, seff;

  if (p >= 0.0)
    return 1.0;
  else
  {
    n = 1.0 / (1.0 - m);
    seff = 1.0 + std::pow(- al * p, n);
    return std::pow(seff, - m);
  }
}

Real
dseff(Real p, Real al, Real m)
{
  if (p >= 0.0)
    return 0.0;
  else
  {
    Real n = 1.0 / (1.0 - m);
    Real inner = 1.0 + std::pow(- al * p, n);
    Real dinner_dp = - n * al * std::pow( - al * p, n - 1.0);
    Real dseff_dp = - m * std::pow(inner, - m - 1) * dinner_dp;
    return dseff_dp;
  }
}

Real
d2seff(Real p, Real al, Real m)
{
  if (p >= 0.0)
    return 0.0;
  else
  {
    Real n = 1.0 / (1.0 - m);
    Real inner = 1.0 + std::pow( - al * p, n);
    Real dinner_dp = - n * al * std::pow(- al * p, n - 1.0);
    Real d2inner_dp2 = n * (n - 1.0) * al * al * std::pow(- al * p, n - 2.0);
    Real d2seff_dp2 = m * (m + 1.0) * std::pow(inner, - m - 2.0) * std::pow(dinner_dp, 2.0) - m * std::pow(inner, - m - 1.0) * d2inner_dp2;
    return d2seff_dp2;
  }
}

Real
relativePermeability(Real seff, Real m)
{
  if (seff <= 0)
    return 0.0;
  else if (seff >= 1.0)
    return 1.0;

  const Real a = 1.0 - std::pow(seff, 1.0 / m);
  const Real b = 1.0 - std::pow(a, m);
  return std::sqrt(seff) * Utility::pow<2>(b);
}

Real
drelativePermeability(Real seff, Real m)
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
   return 0.0;

  const Real a = 1.0 - std::pow(seff, 1.0 / m);
  const Real da = - 1.0 / m * std::pow(seff, 1.0 / m - 1.0);
  const Real b = 1.0 - std::pow(a, m);
  const Real db = - m * std::pow(a, m - 1.0) * da;
  return 0.5 * std::pow(seff, - 0.5) * Utility::pow<2>(b) + 2 * std::sqrt(seff) * b * db;
}

Real
d2relativePermeability(Real seff, Real m)
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
   return 0.0;

  const Real a = 1.0 - std::pow(seff, 1.0 / m);
  const Real da = - 1.0 / m * std::pow(seff, 1.0 / m - 1.0);
  const Real d2a = - (1.0 / m) * (1.0 / m - 1.0) * std::pow(seff, 1.0 / m - 2.0);
  const Real b = 1.0 - std::pow(a, m);
  const Real db = - m * std::pow(a, m - 1.0) * da;
  const Real d2b = - m * (m - 1.0) * std::pow(a, m - 2.0) * da * da - m * std::pow(a, m - 1.0) * d2a;
  return - 0.25 * std::pow(seff, -1.5) * Utility::pow<2>(b) + 2 * std::pow(seff, - 0.5) * b * db + 2 * std::sqrt(seff) * db * db + 2 * std::sqrt(seff) * b * d2b;
}
}
