//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelValue.h"
#include "KokkosFunction.h"

class KokkosBodyForce : public Moose::Kokkos::KernelValue
{
public:
  static InputParameters validParams();

  KokkosBodyForce(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;

protected:
  /// Scale factor
  const Moose::Kokkos::Scalar<const Real> _scale;

  /// Optional function value
  const Moose::Kokkos::Function _function;

  /// Optional Postprocessor value
  const Moose::Kokkos::PostprocessorValue _postprocessor;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBodyForce::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return -_scale * _postprocessor * _function.value(_t, datum.q_point(qp));
}
