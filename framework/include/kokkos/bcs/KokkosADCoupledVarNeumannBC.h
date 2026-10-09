//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADIntegratedBC.h"

/**
 * Implements a Neumann BC where grad(u) = scale_factor * coef * v on the boundary, with its
 * Jacobian computed by pointwise automatic differentiation
 */
class KokkosADCoupledVarNeumannBC : public Moose::Kokkos::ADIntegratedBC
{
public:
  /// The residual depends on the values of v and of the scale factor
  static constexpr unsigned int max_ad_slots = 2;

  static InputParameters validParams();

  KokkosADCoupledVarNeumannBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal<max_ad_slots>
  precomputeQpResidualValue(const unsigned int qp, AssemblyDatum & datum) const
  {
    return -_scale_factor(datum, qp) * _coef * _coupled_var(datum, qp);
  }

protected:
  /// Variable providing the value of grad(u) on the boundary
  const Moose::Kokkos::ADVariableValue<max_ad_slots> _coupled_var;

  /// A coefficient that is multiplied with the residual contribution
  const Real _coef;

  /// Scale factor
  const Moose::Kokkos::ADVariableValue<max_ad_slots> _scale_factor;
};
