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

/**
 * Adds a source term coupled to a scalar variable.
 */
class KokkosScalarVarKernel : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosScalarVarKernel(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidualValue(const unsigned int /* qp */,
                                                 AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpOffDiagJacobianScalarValue(const unsigned int /* j */,
                                                              const unsigned int jvar,
                                                              const unsigned int /* qp */,
                                                              AssemblyDatum & /* datum */) const;

protected:
  const unsigned int _alpha_var;
  const Moose::Kokkos::VariableValue _alpha;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosScalarVarKernel::precomputeQpResidualValue(const unsigned int /* qp */,
                                                 AssemblyDatum & datum) const
{
  return -_alpha(datum, 0);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosScalarVarKernel::precomputeQpOffDiagJacobianScalarValue(const unsigned int /* j */,
                                                              const unsigned int jvar,
                                                              const unsigned int /* qp */,
                                                              AssemblyDatum & /* datum */) const
{
  return jvar == _alpha_var ? -1 : 0;
}
