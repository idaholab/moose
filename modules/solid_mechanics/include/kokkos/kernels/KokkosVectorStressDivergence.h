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
 *
 * @tparam RankTwo The second-order tensor representation of the stress
 * @tparam RankFour The fourth-order tensor representation of the tangent
 *
 * The representation is a template parameter because a minor-symmetric tangent in Mandel notation
 * contracts in 36 multiplies where the dense form takes 81, and because it is the representation
 * NEML2 produces. Both instantiations compute the same operator: the dense form contracts the
 * tangent with the unsymmetrized shape function gradient, matching the non-Kokkos
 * StressDivergenceTensors, and the symmetric form contracts with its symmetric part, which agrees
 * whenever the tangent has minor symmetry.
 */
template <typename RankTwo, typename RankFour>
class KokkosVectorStressDivergenceTempl : public Moose::Kokkos::VectorKernelGrad
{
public:
  static InputParameters validParams();

  KokkosVectorStressDivergenceTempl(const InputParameters & parameters);

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
  const Moose::Kokkos::MaterialProperty<RankTwo> _stress;
  /// Derivative of the stress with respect to the strain
  const Moose::Kokkos::MaterialProperty<RankFour> _Jacobian_mult;
};

typedef KokkosVectorStressDivergenceTempl<Moose::Kokkos::Real33, Moose::Kokkos::Real3333>
    KokkosVectorStressDivergence;
typedef KokkosVectorStressDivergenceTempl<Moose::Kokkos::Real6, Moose::Kokkos::Real66>
    KokkosSymmetricVectorStressDivergence;

template <typename RankTwo, typename RankFour>
template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosVectorStressDivergenceTempl<RankTwo, RankFour>::precomputeQpResidual(
    const unsigned int qp, AssemblyDatum & datum) const
{
  const RankTwo & stress = _stress(datum, qp);

  return stress.full();
}

template <typename RankTwo, typename RankFour>
template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosVectorStressDivergenceTempl<RankTwo, RankFour>::precomputeQpJacobian(
    const unsigned int j, const unsigned int qp, AssemblyDatum & datum) const
{
  const RankFour & Jacobian_mult = _Jacobian_mult(datum, qp);

  return (Jacobian_mult * RankTwo(_grad_phi(datum, j, qp))).full();
}
