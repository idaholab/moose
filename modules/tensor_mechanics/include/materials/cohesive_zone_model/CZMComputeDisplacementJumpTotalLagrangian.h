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
class CZMComputeDisplacementJumpTotalLagrangian : public CZMComputeDisplacementJumpBase
{
public:
  static InputParameters validParams();
  CZMComputeDisplacementJumpTotalLagrangian(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;

  /// compute the interface displacement increment using an incremental
  /// total Lagrangian approach
  void computeLocalDisplacementJump() override;

  /// method computing F and the associated rotation
  void computeFandR();

  /// the old displacement jump in global and interface coordiantes
  ///@{
  const MaterialProperty<RealVectorValue> & _displacement_jump_global_old;
  const MaterialProperty<RealVectorValue> & _interface_displacement_jump_old;
  ///@}

  /// the displacement jump increments in global and interface coordiantes
  ///@{
  MaterialProperty<RealVectorValue> & _displacement_jump_global_inc;
  MaterialProperty<RealVectorValue> & _interface_displacement_jump_inc;
  ///@}

  /// the coupled displacement and neighbor displacement gradient
  ///@{
  std::vector<const VariableGradient *> _grad_disp;
  std::vector<const VariableGradient *> _grad_disp_neighbor;
  ///@}

  /// the interface deformation gradient
  ///@{
  MaterialProperty<RankTwoTensor> & _F;
  const MaterialProperty<RankTwoTensor> & _F_old;
  ///@}

  /// the interface rotation caused by deformation and rigid body motion
  ///@{
  MaterialProperty<RankTwoTensor> & _R;
  const MaterialProperty<RankTwoTensor> & _R_old;
  ///@}

  /// the total interface rotation and its increment (Q=R*Q0)
  ///@{
  MaterialProperty<RankTwoTensor> & _czm_total_rotation;
  MaterialProperty<RankTwoTensor> & _czm_total_rotation_inc;
  ///@}
};
