//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"
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
class SCZMComputeDisplacementJumpBase : public InterfaceMaterial
{
public:
  static InputParameters validParams();
  SCZMComputeDisplacementJumpBase(const InputParameters & parameters);

protected:
  void computeQpProperties() override;
  void initQpStatefulProperties() override;

  virtual void initialSetup() override final;

  /// method used to compute the disaplcement jump in interface coordinates according to a
  ///  specific kinematic formulation
  virtual void computeLocalDisplacementJump() = 0;

  /// method computing the required rotation matrices
  virtual void computeRotationMatrices();

  /// Base name of the material system
  const std::string _base_name;

  /// number of displacement components
  const unsigned int _ndisp;

  /// the coupled displacement and neighbor displacement values
  ///@{
  std::vector<const GenericVariableValue<is_ad> *> _disp;
  std::vector<const GenericVariableValue<is_ad> *> _disp_neighbor;
  ///@}

  /// the coupled displacement gradients
  ///@{
  std::vector<const GenericVariableGradient<is_ad> *> _grad_disp;
  std::vector<const GenericVariableGradient<is_ad> *> _grad_disp_neighbor;
  ///@}

  /// the displacement jump in global and interface coordiantes
  ///@{
  GenericMaterialProperty<RealVectorValue, is_ad> & _displacement_jump_global;
  GenericMaterialProperty<RealVectorValue, is_ad> & _interface_displacement_jump;
  ///@}

  /// the rotation matrix transforming from the interface to the global coordinate systems
  GenericMaterialProperty<RankTwoTensor, is_ad> & _czm_total_rotation;

  /// @brief  Flag indicating whether shifted terms are included
  bool _shifted;

  /// Distance/normal provider user object
  const BoundaryShortestDistanceToSurface * _sbm_distance_uo = nullptr;
};
