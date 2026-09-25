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
 * Stress computed from a small strain through linear elasticity.
 *
 * The tangent equals the elasticity tensor, so it is stored at that tensor's granularity rather than
 * per quadrature point. For the common case of one set of elastic constants per block that is one
 * rank-four tensor per subdomain instead of one per quadrature point.
 */
class KokkosComputeLinearElasticStress : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosComputeLinearElasticStress(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION void computeSubdomainProperties(const unsigned int qp, Datum & datum) const
  {
    _Jacobian_mult(datum, qp) = _elasticity_tensor(datum, qp);
  }

  /**
   * Verify that the elasticity tensor is stored at the granularity the tangent is declared with.
   * Deferred past construction because MOOSE constructs materials in input order, so the elasticity
   * tensor's declaration may not have happened yet.
   */
  void checkElasticityTensorGranularity();

private:
  /// Base name prefixing the property names
  const std::string _base_name;

  /// Elasticity tensor
  const Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real3333> _elasticity_tensor;
  /// Mechanical strain
  const Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real33> _mechanical_strain;

  /// Stress
  Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real33> _stress;
  /// Derivative of the stress with respect to the strain, stored per subdomain
  Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real3333> _Jacobian_mult;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosComputeLinearElasticStress::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  const Moose::Kokkos::Real3333 & elasticity_tensor = _elasticity_tensor(datum, qp);
  const Moose::Kokkos::Real33 & mechanical_strain = _mechanical_strain(datum, qp);

  _stress(datum, qp) = elasticity_tensor * mechanical_strain;
}
