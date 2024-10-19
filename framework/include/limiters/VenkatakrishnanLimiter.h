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
class VenkatakrishnanLimiter : public Limiter<T>
{
public:
  T limit(const T & phi_upwind,
          const T &,
          const VectorValue<T> * grad_phi_upwind,
          const VectorValue<T> *,
          const RealVectorValue &,
          const T & max_value,
          const T & min_value,
          const FaceInfo * fi,
          const bool & fi_elem_is_upwind) const override final
  {
    const auto face_centroid = fi->faceCentroid();
    const auto cell_centroid = fi_elem_is_upwind ? fi->elemCentroid() : fi->neighborCentroid();

    const auto delta_face = (*grad_phi_upwind) * (face_centroid - cell_centroid);
    const auto delta_max = max_value - phi_upwind + 1e-10;
    const auto delta_min = min_value - phi_upwind + 1e-10;

    return delta_face >= 0 ? delta_face / delta_max : delta_face / delta_min;

  }
  bool constant() const override final { return false; }
  InterpMethod interpMethod() const override final { return InterpMethod::SOU; }

  VenkatakrishnanLimiter() = default;
};
}
}
