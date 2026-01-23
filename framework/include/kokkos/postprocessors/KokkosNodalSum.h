//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalVariablePostprocessor.h"

class KokkosNodalSum : public KokkosNodalVariablePostprocessor
{
public:
  static InputParameters validParams();

  KokkosNodalSum(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

  template <typename Derived>
  KOKKOS_FUNCTION void
  executeShim(const Derived & postprocessor, Datum & datum, Real * result) const;

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, Datum & datum) const
  {
    return _u(datum, qp);
  }

  KOKKOS_FUNCTION void join(DefaultLoop, Real * result, const Real * source) const;
  KOKKOS_FUNCTION void init(DefaultLoop, Real * result) const;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosNodalSum::executeShim(const Derived & postprocessor, Datum & datum, Real * result) const
{
  if (datum.isNodalDefined(_u.variable()))
    result[0] += postprocessor.computeValue(0, datum);
}

KOKKOS_FUNCTION inline void
KokkosNodalSum::join(DefaultLoop, Real * result, const Real * source) const
{
  result[0] += source[0];
}

KOKKOS_FUNCTION inline void
KokkosNodalSum::init(DefaultLoop, Real * result) const
{
  result[0] = 0;
}
