//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementStatistics.h"

class KokkosElementVariableStatistics : public KokkosElementStatistics
{
public:
  static InputParameters validParams();

  KokkosElementVariableStatistics(const InputParameters & parameters);

  KOKKOS_FUNCTION Kokkos::pair<Real, Real> computeValue(Datum & datum) const;

protected:
  /// The coupled variable used.
  const Moose::Kokkos::VariableValue _v;
};

KOKKOS_FUNCTION inline Kokkos::pair<Real, Real>
KokkosElementVariableStatistics::computeValue(Datum & datum) const
{
  Real val = 0;
  Real vol = 0;

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    datum.reinit();

    val += _v(datum, qp) * datum.JxW(qp);
    vol += datum.JxW(qp);
  }

  val /= vol;

  return {val, vol};
}
