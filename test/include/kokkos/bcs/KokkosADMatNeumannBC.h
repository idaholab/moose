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
 * Test AD integrated BC with the flux k u given by a pointwise AD material property k, whose
 * derivatives enter the Jacobian computed by pointwise automatic differentiation:
 *
 *   (psi_i, -k u)
 */
class KokkosADMatNeumannBC : public Moose::Kokkos::ADIntegratedBC
{
public:
  /// u, followed by the inputs of k coupled to u: one coupled field value and gradient
  static constexpr unsigned int max_ad_slots = 5;

  static InputParameters validParams();

  KokkosADMatNeumannBC(const InputParameters & parameters);

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal<slots>
  precomputeQpResidualValue(const unsigned int qp,
                            Moose::Kokkos::ADAssemblyDatum<slots> & datum) const
  {
    return -_k(datum, qp) * _u(datum, qp);
  }

private:
  /// The BC variable
  const Moose::Kokkos::ADVariableValue _u;
  /// The property multiplying the BC variable in the flux
  const Moose::Kokkos::ADMaterialPropertyReader _k;
};
