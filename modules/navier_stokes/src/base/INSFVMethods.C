//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMethods.h"

ADReal
findUStar(const Real mu, const Real rho, ADReal u, const Real dist)
{
  constexpr int MAX_ITERS{50};
  constexpr Real REL_TOLERANCE{1e-6};

  constexpr Real von_karman{0.4187};

  Real nu = mu / rho;

  ADReal u_star = std::sqrt(nu * u / dist);

  // Newton-Raphson method to solve for u_star (friction velocity).
  for (int i = 0; i < MAX_ITERS; ++i)
  {
    ADReal residual = u_star / von_karman * std::log(u_star * dist / (0.111 * nu)) - u;
    ADReal deriv = (1 + std::log(u_star * dist / (0.111 * nu))) / von_karman;
    ADReal new_u_star = u_star - residual / deriv;

    ADReal rel_err = std::abs(new_u_star - u_star) / new_u_star;
    u_star = new_u_star;
    if (rel_err < REL_TOLERANCE)
      return u_star;
  }

  mooseError("Could not find the wall friction velocity (mu: ",
             mu,
             " rho: ",
             rho,
             " velocity: ",
             u,
             " wall distance: ",
             dist,
             ")");
}
