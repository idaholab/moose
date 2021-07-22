//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Limiter.h"
#include "FVUtils.h"

namespace Moose
{
namespace FV
{
/**
 * Implements a limiter which reproduces the second-order-upwind scheme, defined by
 * $\beta(r_f) = r_f$
 */
class SOULimiter : public Limiter
{
public:
  ADReal operator()(const ADReal & phi_upwind,
                    const ADReal & phi_downwind,
                    const ADRealVectorValue * grad_phi_upwind,
                    const RealVectorValue & dCD) const override final
  {
    mooseAssert(grad_phi_upwind, "min-mod limiter requires a gradient");
    const auto r_f = Moose::FV::rF(phi_upwind, phi_downwind, *grad_phi_upwind, dCD);
    return r_f;
  }
  bool constant() const override final { return false; }

  SOULimiter() = default;
};
}
}
