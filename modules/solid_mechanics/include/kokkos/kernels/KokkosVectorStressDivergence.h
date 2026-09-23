//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorKernelGrad.h"

/**
 * Stress divergence for a single vector displacement variable, with the weak form
 * $(\sigma, \nabla \vec{\psi_i})$.
 *
 * The constitutive model is supplied entirely through the stress and its derivative with respect
 * to the strain, so this kernel is independent of the material model producing them.
 */
class KokkosVectorStressDivergence : public Moose::Kokkos::VectorKernelGrad
{
public:
  static InputParameters validParams();

  KokkosVectorStressDivergence(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real33 precomputeQpResidual(const unsigned int qp,
                                                             AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real33
  precomputeQpJacobian(const unsigned int j, const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Base name prefixing the consumed property names
  const std::string _base_name;

  /// Stress
  const Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real33> _stress;
  /// Derivative of the stress with respect to the strain
  const Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real3333> _Jacobian_mult;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosVectorStressDivergence::precomputeQpResidual(const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  return _stress(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosVectorStressDivergence::precomputeQpJacobian(const unsigned int j,
                                                   const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  // Contracting the tangent with the unsymmetrized shape function gradient matches what the
  // non-Kokkos StressDivergenceTensors does through ElasticityTensorTools::elasticJacobian, and
  // agrees with the symmetrized form whenever the tangent has minor symmetry.
  const Moose::Kokkos::Real3333 & Jacobian_mult = _Jacobian_mult(datum, qp);

  return Jacobian_mult * _grad_phi(datum, j, qp);
}
