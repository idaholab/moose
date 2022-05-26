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
#include "MathFVUtils.h"

namespace Moose
{
namespace FV
{
/**
 * Implements the Min-Mod limiter, defined by
 * $\beta(r_f) = \text{max}(0, \text{min}(1, r_f))$
 */
template <typename T>
class MinModLimiter : public Limiter<T>
{
public:
  T limit(const T & phi_upwind,
          const T & phi_downwind,
          const VectorValue<T> * grad_phi_upwind,
          const RealVectorValue & dCD) const override final
  {
    mooseAssert(grad_phi_upwind, "min-mod limiter requires a gradient");
    const auto r_f = Moose::FV::rF(phi_upwind, phi_downwind, *grad_phi_upwind, dCD);

    // Dummy addition to avoid new nonzeros
    return 0 * r_f + std::max(T(0), std::min(T(1), r_f));
  }
  bool constant() const override final { return false; }
  InterpMethod interpMethod() const override final { return InterpMethod::MinMod; }

  MinModLimiter() = default;
};
}
}
