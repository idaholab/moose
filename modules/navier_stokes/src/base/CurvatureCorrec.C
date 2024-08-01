//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CurvatureCorrec.h"
#include "MooseError.h"
#include "libmesh/vector_value.h"
#include "NS.h"

namespace CurvatureCorrec
{

/// Function that finds the velocity in the swirling direction due to curvature
ADReal
findWStar(const ADReal mu,
          const ADReal rho,
          const ADReal & w,
          const Real dist,
          const ADReal curv_R,
          const bool convex)
{
  // usually takes about 3-4 iterations
  constexpr int MAX_ITERS{50};
  constexpr Real REL_TOLERANCE{1e-6};

  const ADReal nu = mu / rho;

  ADReal w_star = std::sqrt(nu * w / dist);

  if (convex)
  {
    // Newton-Raphson method to solve for u_star (friction velocity).
    for (int i = 0; i < MAX_ITERS; ++i)
    {
      ADReal residual =
          w / w_star - (1 + dist / curv_R) *
                           (1 / NS::von_karman_constant *
                            log(NS::E_turb_constant * dist * w_star / nu / (1 + dist / curv_R)));

      ADReal deriv = -1 / w_star * (w / w_star + (1 + dist / curv_R) / NS::von_karman_constant);

      ADReal new_w_star = std::max(1e-20, w_star - residual / deriv);

      Real rel_err = std::abs((new_w_star.value() - w_star.value()) / new_w_star.value());

      w_star = new_w_star;
      if (rel_err < REL_TOLERANCE)
        break;
    }
    return w_star;
  }

  // Newton-Raphson method to solve for u_star (friction velocity).
  for (int i = 0; i < MAX_ITERS; ++i)
  {
    ADReal residual =
        w / w_star -
        (1 - dist / curv_R) * (1 / NS::von_karman_constant *
                               log(NS::E_turb_constant * dist * w_star / nu / (1 - dist / curv_R)));

    ADReal deriv = -1 / w_star * (w / w_star + (1 - dist / curv_R) / NS::von_karman_constant);

    ADReal new_w_star = std::max(1e-20, w_star - residual / deriv);

    Real rel_err = std::abs((new_w_star.value() - w_star.value()) / new_w_star.value());

    w_star = new_w_star;
    if (rel_err < REL_TOLERANCE)
      break;
  }
  return w_star;
}

}
