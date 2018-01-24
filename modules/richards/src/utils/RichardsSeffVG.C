//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  van-Genuchten effective saturation as a function of pressure (not capillary pressure)
//
#include "RichardsSeffVG.h"

Real
RichardsSeffVG::seff(Real p, Real al, Real m)
{
  Real n, seff;

  if (p >= 0)
    return 1.0;
  else
  {
    n = 1.0 / (1.0 - m);
    seff = 1 + std::pow(-al * p, n);
    return std::pow(seff, -m);
  }
}

Real
RichardsSeffVG::dseff(Real p, Real al, Real m)
{
  if (p >= 0)
    return 0.0;
  else
  {
    Real n = 1.0 / (1.0 - m);
    Real inner = 1 + std::pow(-al * p, n);
    Real dinner_dp = -n * al * std::pow(-al * p, n - 1);
    Real dseff_dp = -m * std::pow(inner, -m - 1) * dinner_dp;
    return dseff_dp;
  }
}

Real
RichardsSeffVG::d2seff(Real p, Real al, Real m)
{
  if (p >= 0)
    return 0.0;
  else
  {
    Real n = 1.0 / (1.0 - m);
    Real inner = 1 + std::pow(-al * p, n);
    Real dinner_dp = -n * al * std::pow(-al * p, n - 1);
    Real d2inner_dp2 = n * (n - 1) * al * al * std::pow(-al * p, n - 2);
    Real d2seff_dp2 = m * (m + 1) * std::pow(inner, -m - 2) * std::pow(dinner_dp, 2) -
                      m * std::pow(inner, -m - 1) * d2inner_dp2;
    return d2seff_dp2;
  }
}
