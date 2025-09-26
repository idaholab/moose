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
 * This calculates the time derivative for a coupled variable
 **/
class KokkosCoupledTimeDerivative : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosCoupledTimeDerivative(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int jvar,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const;

protected:
  const Moose::Kokkos::VariableValue _v_dot;
  const Moose::Kokkos::Scalar<const Real> _dv_dot;
  const unsigned int _v_var;
};

KOKKOS_FUNCTION inline Real
KokkosCoupledTimeDerivative::computeQpResidual(const unsigned int i,
                                               const unsigned int qp,
                                               ResidualDatum & datum) const
{
  return _test(datum, i, qp) * _v_dot(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosCoupledTimeDerivative::computeQpOffDiagJacobian(const unsigned int i,
                                                      const unsigned int j,
                                                      const unsigned int jvar,
                                                      const unsigned int qp,
                                                      ResidualDatum & datum) const
{
  if (jvar == _v_var)
    return _test(datum, i, qp) * _phi(datum, j, qp) * _dv_dot;
  else
    return 0;
}
