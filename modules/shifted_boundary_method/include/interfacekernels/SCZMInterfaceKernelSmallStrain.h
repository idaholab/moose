//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCZMInterfaceKernelBase.h"

/// Non-AD DG cohesive zone model kernel for the small strain formulation. This is the
/// hand-coded-Jacobian counterpart of ADSCZMInterfaceKernelSmallStrain and assumes the
/// traction-separation law only depends on the displacement jump. One kernel is required for each
/// displacement component.
class SCZMInterfaceKernelSmallStrain : public SCZMInterfaceKernelBase
{
public:
  static InputParameters validParams();
  SCZMInterfaceKernelSmallStrain(const InputParameters & parameters);

protected:
  Real computeQpResidual(Moose::DGResidualType type) override;
  Real computeQpJacobian(Moose::DGJacobianType type) override;
  Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  Real computeDResidualDDisplacement(const unsigned int & component_j,
                                     const Moose::DGJacobianType & type) const override;

  /// Analytic Jacobian of the stress-based directional correction term
  Real calculateDirectionalCorrectionJacobian(unsigned int ivar,
                                               unsigned int jvar,
                                               Moose::DGJacobianType type) const;

  /// The stress tensor on the element and neighbor sides
  ///@{
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _stress_neighbor;
  ///@}

  /// The material tangent on the element and neighbor sides
  ///@{
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;
  const MaterialProperty<RankFourTensor> & _Jacobian_mult_neighbor;
  ///@}

  /// Whether the material tangent is d(PK1)/d(F) instead of d(stress)/d(strain)
  bool _tangent_is_dpk1_df;

  /// Whether to add the directional correction term
  const bool _directional_correction;

  /// Whether to apply the volumetric locking correction to the directional correction term
  const bool _volumetric_locking_correction;
};
