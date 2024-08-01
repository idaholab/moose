//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RoughnessCorrec.h"
#include "MooseError.h"
#include "libmesh/vector_value.h"
#include "NS.h"

namespace RoughnessCorrec

{

/// Function that computes the friction velocity for a rough wall
ADReal
roughness_correc_UStar(
    const ADReal & mu, const ADReal & rho, const ADReal & u, const Real dist, const Real ks)
{
  // usually takes about 3-4 iterations
  constexpr int MAX_ITERS{50};
  constexpr Real REL_TOLERANCE{1e-6};

  const ADReal nu = mu / rho;

  ADReal u_star = std::sqrt(nu * u / dist);

  // Newton-Raphson method to solve for u_star (friction velocity).
  for (int i = 0; i < MAX_ITERS; ++i)
  {
    ADReal residual =
        u_star / NS::von_karman_constant *
            std::log(NS::E_turb_constant * dist / (nu / u_star + NS::C_rough_constant * ks)) -
        u;
    ADReal deriv =
        (1 + std::log(NS::E_turb_constant * dist / (nu / u_star + NS::C_rough_constant * ks)) -
         1 / (1 + nu / (NS::C_rough_constant * ks * u_star))) /
        NS::von_karman_constant;
    ADReal new_u_star = std::max(1e-20, u_star - residual / deriv);

    Real rel_err = std::abs((new_u_star.value() - u_star.value()) / new_u_star.value());

    u_star = new_u_star;
    if (rel_err < REL_TOLERANCE)
      return u_star;
  }
  return u_star;
}
}
