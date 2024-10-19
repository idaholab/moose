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
 * Implements a limiter which reproduces the QUICK scheme, defined by
 * $\beta(r_f) = \frac{3+r_f}{4}$
 */
template <typename T>
class QUICKLimiter : public Limiter<T>
{
public:
  T limit(const T & phi_upwind,
          const T & phi_downwind,
          const VectorValue<T> * grad_phi_upwind,
          const VectorValue<T> * grad_phi_downwind,
          const RealVectorValue & dCD,
          const T &,
          const T &,
          const FaceInfo * fi,
          const bool & fi_elem_is_upwind) const override final
  {
    mooseAssert(grad_phi_upwind, "QUICK limiter requires a gradient");

    const auto & w_f = fi_elem_is_upwind ? fi->gC() : (1. - fi->gC());
    const auto & phiCD = w_f * phi_upwind + (1.0 - w_f) * phi_downwind;
    const auto & face_grad = w_f * (*grad_phi_upwind) + (1.0 - w_f) * (*grad_phi_downwind);
    const auto & faceFlux = face_grad * fi->normal();

    T phiU, phif;

    if (faceFlux > 0)
    {
      phiU = phi_upwind;
      phif = 0.5 * (phiCD + phiU + (1 - w_f) * (dCD * (*grad_phi_upwind)));
    }
    else
    {
      phiU = phi_downwind;
      phif = 0.5 * (phiCD + phiU - w_f * (dCD * (*grad_phi_downwind)));
    }

    const auto & s = phiCD - phiU;
    if (s >= 0)
    {
      return s + 1e-10;
    }
    else
    {
      return s - 1e-10;
    }

    // Calculate the effective limiter for the QUICK interpolation
    const auto quick = (phif - phiU) / s;

    // Limit the limiter between upwind and downwind
    return std::max(std::min(quick, T(2)), T(0));
  }
  bool constant() const override final { return false; }
  InterpMethod interpMethod() const override final { return InterpMethod::QUICK; }

  QUICKLimiter() = default;
};
}
}
