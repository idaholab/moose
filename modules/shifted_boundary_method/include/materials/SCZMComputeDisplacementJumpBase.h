//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeDisplacementJumpBase.h"
#include "BoundaryShortestDistanceToSurface.h"

#define usingSCZMComputeDisplacementJumpBaseMembers                                                \
  usingInterfaceMaterialMembers;                                                                   \
  using SCZMComputeDisplacementJumpBase<is_ad>::_base_name;                                        \
  using SCZMComputeDisplacementJumpBase<is_ad>::_ndisp;                                            \
  using SCZMComputeDisplacementJumpBase<is_ad>::_disp;                                             \
  using SCZMComputeDisplacementJumpBase<is_ad>::_disp_neighbor;                                    \
  using SCZMComputeDisplacementJumpBase<is_ad>::_displacement_jump_global;                         \
  using SCZMComputeDisplacementJumpBase<is_ad>::_interface_displacement_jump;                      \
  using SCZMComputeDisplacementJumpBase<is_ad>::_czm_total_rotation

/**
 * This interface material class computes the displacement jump in the interface natural coordinate
 * system. The transformation between local and global coordinates shall be defined in
 * computeLocalDisplacementJump.
 */
template <bool is_ad>
class SCZMComputeDisplacementJumpBase : public CZMComputeDisplacementJumpBase<is_ad>
{
public:
  static InputParameters validParams();
  SCZMComputeDisplacementJumpBase(const InputParameters & parameters);

protected:
  using CZMComputeDisplacementJumpBase<is_ad>::_czm_total_rotation;
  using CZMComputeDisplacementJumpBase<is_ad>::_displacement_jump_global;
  using CZMComputeDisplacementJumpBase<is_ad>::_mesh;
  using CZMComputeDisplacementJumpBase<is_ad>::_ndisp;
  using CZMComputeDisplacementJumpBase<is_ad>::_qp;

  virtual void initialSetup() override final;

  /// Computes the jump at the true interface using displacement gradients.
  void computeGlobalDisplacementJump() override;

  /// Computes the rotation using the true-interface normal.
  void computeRotationMatrices() override;

  /// the coupled displacement gradients
  ///@{
  std::vector<const GenericVariableGradient<is_ad> *> _grad_disp;
  std::vector<const GenericVariableGradient<is_ad> *> _grad_disp_neighbor;
  ///@}

  /// @brief  Flag indicating whether shifted terms are included
  bool _shifted;

  /// Distance/normal provider user object
  const BoundaryShortestDistanceToSurface * _sbm_distance_uo = nullptr;
};
