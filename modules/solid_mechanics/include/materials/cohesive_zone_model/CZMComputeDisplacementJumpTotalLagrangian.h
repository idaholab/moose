//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeDisplacementJumpBase.h"

/**
 * Compute the displacement jump in interface coordinates across a cohesive zone for the total
 * Lagrangian formulation.
 */
template <bool is_ad>
class CZMComputeDisplacementJumpTotalLagrangianTempl : public CZMComputeDisplacementJumpBase<is_ad>
{
public:
  static InputParameters validParams();
  CZMComputeDisplacementJumpTotalLagrangianTempl(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;

  /// compute the interface displacement increment using an incremental
  /// total Lagrangian approach
  void computeLocalDisplacementJump() override;

  /// method computing the required rotation matrices
  void computeRotationMatrices() override;

  /// method computing F and the associated rotation
  void computeFandR();

  /// the coupled displacement and neighbor displacement gradient
  ///@{
  std::vector<const GenericVariableGradient<is_ad> *> _grad_disp;
  std::vector<const GenericVariableGradient<is_ad> *> _grad_disp_neighbor;
  ///@}

  /// the interface deformation gradient
  GenericMaterialProperty<RankTwoTensor, is_ad> & _F;

  /// the interface rotation caused by deformation and rigid body motion
  GenericMaterialProperty<RankTwoTensor, is_ad> & _R;

  /// the rotation matrix transforming from local to global coordinates in the undeformed configuration
  GenericMaterialProperty<RankTwoTensor, is_ad> & _czm_reference_rotation;

  usingCZMComputeDisplacementJumpBaseMembers;
};

typedef CZMComputeDisplacementJumpTotalLagrangianTempl<false>
    CZMComputeDisplacementJumpTotalLagrangian;
typedef CZMComputeDisplacementJumpTotalLagrangianTempl<true>
    ADCZMComputeDisplacementJumpTotalLagrangian;
