//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"

class KokkosBodyForce : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosBodyForce(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const;

protected:
  /// Scale factor
  const Moose::Kokkos::Scalar<const Real> _scale;

  /// Optional Postprocessor value
  const Moose::Kokkos::PostprocessorValue _postprocessor;
};

KOKKOS_FUNCTION inline Real
KokkosBodyForce::computeQpResidual(const unsigned int i,
                                   const unsigned int qp,
                                   ResidualDatum & datum) const
{
  return -_test(datum, i, qp) * _scale * _postprocessor;
}
