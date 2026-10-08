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

/**
 * Kokkos volumetric heat source term
 */
class KokkosHeatSource : public Moose::Kokkos::KernelValue
{
public:
  static InputParameters validParams();

  KokkosHeatSource(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Scale factor
  const Moose::Kokkos::Scalar<const Real> _scale;
  /// Postprocessor multiplying the heat source
  const Moose::Kokkos::PostprocessorValue _postprocessor;
  /// Function describing the volumetric heat source
  const Moose::Kokkos::Function _function;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosHeatSource::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return -_scale * _postprocessor * _function.value(_t, datum.q_point(qp));
}
