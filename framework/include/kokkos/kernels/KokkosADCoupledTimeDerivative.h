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
 * The time derivative of a coupled variable $(\psi_i, \partial v_h / \partial t)$ with its
 * Jacobian computed by pointwise automatic differentiation
 */
class KokkosADCoupledTimeDerivative : public Moose::Kokkos::ADKernel
{
public:
  /// The residual depends on the time derivative of v
  static constexpr unsigned int max_ad_slots = 1;

  static InputParameters validParams();

  KokkosADCoupledTimeDerivative(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal<max_ad_slots>
  precomputeQpResidualValue(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _v_dot(datum, qp);
  }

private:
  /// Time derivative of the coupled variable
  const Moose::Kokkos::ADVariableValue<max_ad_slots> _v_dot;
};
