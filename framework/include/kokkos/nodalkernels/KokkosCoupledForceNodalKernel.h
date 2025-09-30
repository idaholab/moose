//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalKernel.h"

class KokkosCoupledForceNodalKernel : public Moose::Kokkos::NodalKernel
{
public:
  static InputParameters validParams();

  KokkosCoupledForceNodalKernel(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, ResidualDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int jvar,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const;

private:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  const Moose::Kokkos::VariableValue _v;

  /// A multiplicative factor for computing the coupled force
  const Real _coef;
};

KOKKOS_FUNCTION inline Real
KokkosCoupledForceNodalKernel::computeQpResidual(const unsigned int qp, ResidualDatum & datum) const
{
  return -_coef * _v(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosCoupledForceNodalKernel::computeQpOffDiagJacobian(const unsigned int jvar,
                                                        const unsigned int /* qp */,
                                                        ResidualDatum & /* datum */) const
{
  if (jvar == _v_var)
    return -_coef;
  return 0;
}
