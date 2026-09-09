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
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class KokkosScalarVarBC : public Moose::Kokkos::IntegratedBC
{
public:
  static InputParameters validParams();

  KokkosScalarVarBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const
  {
    return -_alpha(datum, 0) * _test(datum, i, qp);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobianScalar(const unsigned int i,
                                                      const unsigned int /* j */,
                                                      const unsigned int jvar,
                                                      const unsigned int qp,
                                                      AssemblyDatum & datum) const
  {
    return jvar == _alpha_var ? -_test(datum, i, qp) : 0;
  }

protected:
  const unsigned int _alpha_var;
  const Moose::Kokkos::VariableValue _alpha;
};
