//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADTimeKernel.h"

/**
 * The time derivative operator $(\psi_i, \partial u_h / \partial t)$ with its Jacobian computed
 * by pointwise automatic differentiation
 */
class KokkosADTimeDerivative : public Moose::Kokkos::ADTimeKernel
{
public:
  /// The residual depends on the time derivative of u, whose derivative is carried by the value slot
  static constexpr unsigned int max_ad_slots = 1;

  static InputParameters validParams();

  KokkosADTimeDerivative(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal<max_ad_slots>
  precomputeQpResidualValue(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _u_dot(datum, qp);
  }

private:
  /// Time derivative of the kernel variable
  const Moose::Kokkos::ADVariableValue<max_ad_slots> _u_dot;
};
