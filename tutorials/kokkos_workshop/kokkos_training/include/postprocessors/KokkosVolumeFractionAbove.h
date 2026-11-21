//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementVariablePostprocessor.h"

/**
 * Fraction of the domain volume where a variable exceeds a threshold, computed with the reducer
 * hooks
 */
class KokkosVolumeFractionAbove : public KokkosElementVariablePostprocessor
{
public:
  static InputParameters validParams();

  KokkosVolumeFractionAbove(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

  template <typename Derived>
  KOKKOS_FUNCTION void reduce(Datum & datum, Real * result) const;
  template <typename Derived>
  KOKKOS_FUNCTION void join(Real * result, const Real * source) const;
  template <typename Derived>
  KOKKOS_FUNCTION void init(Real * result) const;

private:
  /// The variable value above which the volume is counted
  const Real _threshold;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosVolumeFractionAbove::reduce(Datum & datum, Real * result) const
{
  // result[0]: volume above the threshold, result[1]: total volume
  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    if (_u(datum, qp) > _threshold)
      result[0] += datum.JxW(qp);
    result[1] += datum.JxW(qp);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosVolumeFractionAbove::join(Real * result, const Real * source) const
{
  result[0] += source[0];
  result[1] += source[1];
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosVolumeFractionAbove::init(Real * result) const
{
  result[0] = 0;
  result[1] = 0;
}
