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
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobianScalar(const unsigned int i,
                                                      const unsigned int j,
                                                      const unsigned int jvar,
                                                      const unsigned int qp,
                                                      AssemblyDatum & datum) const;

protected:
  const unsigned int _alpha_var;
  const Moose::Kokkos::VariableValue _alpha;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosScalarVarKernel::computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const
{
  return -_alpha(datum, 0) * _test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosScalarVarKernel::computeQpOffDiagJacobianScalar(const unsigned int i,
                                                      const unsigned int /* j */,
                                                      const unsigned int jvar,
                                                      const unsigned int qp,
                                                      AssemblyDatum & datum) const
{
  return jvar == _alpha_var ? -_test(datum, i, qp) : 0;
}
