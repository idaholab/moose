//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBC.h"

/**
 * Implements a constant Neumann BC where grad(u) is a equal to a postprocessor on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class KokkosPostprocessorNeumannBC final
  : public Moose::Kokkos::IntegratedBC<KokkosPostprocessorNeumannBC>
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  static InputParameters validParams();

  KokkosPostprocessorNeumannBC(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real
  computeQpResidual(const unsigned int i, const unsigned int qp, ResidualDatum & datum) const;

protected:
  /// Value of grad(u) on the boundary.
  Moose::Kokkos::PostprocessorValue _value;
};

KOKKOS_FUNCTION inline Real
KokkosPostprocessorNeumannBC::computeQpResidual(const unsigned int i,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const
{
  return -_test(datum, i, qp) * _value;
}
