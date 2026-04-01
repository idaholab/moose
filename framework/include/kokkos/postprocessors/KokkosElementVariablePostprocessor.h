//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementPostprocessor.h"

class KokkosElementVariablePostprocessor : public Moose::Kokkos::ElementPostprocessor,
                                           public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  KokkosElementVariablePostprocessor(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void
  executeShim(const Derived & postprocessor, Datum & datum, Real * result) const;

protected:
  /// Holds the solution at current quadrature points
  const Moose::Kokkos::VariableValue _u;

  /// Holds the solution gradient at the current quadrature points
  const Moose::Kokkos::VariableGradient _grad_u;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosElementVariablePostprocessor::executeShim(const Derived & postprocessor,
                                                Datum & datum,
                                                Real * result) const
{
  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    postprocessor.computeQpValue(qp, datum, result);
}
