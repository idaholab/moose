//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalBC.h"

/**
 * Implements a Dirichlet BC where the value on the boundary is equal to a postprocessor value.
 */
class KokkosPostprocessorDirichletBC : public Moose::Kokkos::NodalBC
{
public:
  static InputParameters validParams();

  KokkosPostprocessorDirichletBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _u(datum, qp) - _postprocessor_value;
  }

protected:
  /// The value of the postprocessor to impose on the boundary
  const Moose::Kokkos::PostprocessorValue _postprocessor_value;
};
