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
 * The integrated boundary condition $\partial u / \partial n = h$ with a constant h. Its residual
 * depends on no solution quantity, so it contributes no Jacobian.
 */
class KokkosADNeumannBC : public Moose::Kokkos::ADIntegratedBC
{
public:
  /// The residual depends on no solution quantity
  static constexpr unsigned int max_ad_slots = 0;

  static InputParameters validParams();

  KokkosADNeumannBC(const InputParameters & parameters);

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal<slots>
  precomputeQpResidualValue(const unsigned int /* qp */,
                            Moose::Kokkos::ADAssemblyDatum<slots> & /* datum */) const
  {
    return -_value;
  }

private:
  /// Value of grad(u) on the boundary.
  const Real _value;
};
