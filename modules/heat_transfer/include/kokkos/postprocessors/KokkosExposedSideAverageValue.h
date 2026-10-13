//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAverageValue.h"

/**
 * Kokkos postprocessor computing the average value of a variable on the exposed portion of a
 * sideset, where the illumination state is computed by a KokkosSelfShadowSideUserObject
 */
class KokkosExposedSideAverageValue : public KokkosSideAverageValue
{
public:
  static InputParameters validParams();

  KokkosExposedSideAverageValue(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void reduce(Datum & datum, Real * result) const;

private:
  /// Illumination bitmasks of the local sides shared with the self shadowing user object
  Moose::Kokkos::Array2D<unsigned int> _illumination;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosExposedSideAverageValue::reduce(Datum & datum, Real * result) const
{
  const auto illumination = _illumination(datum.side(), datum.elemID());

  Real sum = 0;
  Real vol = 0;

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    // tests if the bit at position qp is set
    if (illumination & (1u << qp))
    {
      sum += datum.JxW(qp) * computeQpIntegral(qp, datum);
      vol += datum.JxW(qp);
    }

  result[0] += sum;
  result[1] += vol;
}
