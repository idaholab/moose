//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"

/**
 * Constant isotropic elasticity tensor, built from any valid pair of the five isotropic elastic
 * constants.
 */
class KokkosComputeIsotropicElasticityTensor : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosComputeIsotropicElasticityTensor(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Base name prefixing the declared property name
  const std::string _base_name;

  /// Elasticity tensor, constant in space and in time
  Moose::Kokkos::Real3333 _Cijkl;

  /// Elasticity tensor material property
  Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real3333> _elasticity_tensor;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosComputeIsotropicElasticityTensor::computeQpProperties(const unsigned int qp,
                                                            Datum & datum) const
{
  _elasticity_tensor(datum, qp) = _Cijkl;
}
