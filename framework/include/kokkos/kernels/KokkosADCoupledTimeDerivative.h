//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADKernel.h"

/**
 * This calculates the time derivative for a coupled variable
 **/
class KokkosADCoupledTimeDerivative : public Moose::Kokkos::ADKernel
{
public:
  static InputParameters validParams();

  KokkosADCoupledTimeDerivative(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::ADReal
  computeQpResidual(const unsigned int i, const unsigned int qp, AssemblyDatum & datum) const;

protected:
  const Moose::Kokkos::ADVariableValue _v_dot;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::ADReal
KokkosADCoupledTimeDerivative::computeQpResidual(const unsigned int i,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  return _test(datum, i, qp) * _v_dot(datum, qp);
}
