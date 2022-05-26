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
 * Implements a limiter which reproduces the second-order-upwind scheme, defined by
 * $\beta(r_f) = r_f$
 */
template <typename T>
class SOULimiter : public Limiter<T>
{
public:
  T limit(const T & phi_upwind,
          const T & phi_downwind,
          const VectorValue<T> * grad_phi_upwind,
          const RealVectorValue & dCD) const override final
  {
    mooseAssert(grad_phi_upwind, "SOU limiter requires a gradient");
    const auto r_f = Moose::FV::rF(phi_upwind, phi_downwind, *grad_phi_upwind, dCD);
    return r_f;
  }
  bool constant() const override final { return false; }
  InterpMethod interpMethod() const override final { return InterpMethod::SOU; }

  SOULimiter() = default;
};
}
}
