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

/// AD and non-AD shifted cohesive zone interface kernel for the small-strain formulation.
template <bool is_ad>
class SCZMInterfaceKernelSmallStrainTempl : public SCZMInterfaceKernelBaseTempl<is_ad>
{
public:
  static InputParameters validParams();
  SCZMInterfaceKernelSmallStrainTempl(const InputParameters & parameters);

protected:
  GenericReal<is_ad> computeQpResidual(Moose::DGResidualType type) override;
  Real computeQpJacobian(Moose::DGJacobianType type) override;
  Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  Real computeDResidualDDisplacement(unsigned int component_j, Moose::DGJacobianType type) const;
  Real calculateDirectionalCorrectionJacobian(unsigned int ivar,
                                              unsigned int jvar,
                                              Moose::DGJacobianType type) const;

  /// The stress tensor on the element and neighbor sides
  ///@{
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _stress;
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _stress_neighbor;
  ///@}

  /// Non-AD data used by the hand-coded Jacobian
  ///@{
  std::vector<unsigned int> _disp_var;
  std::vector<MooseVariable *> _vars;
  const MaterialProperty<RankTwoTensor> * _dtraction_djump_global;
  const MaterialProperty<RankFourTensor> * _Jacobian_mult;
  const MaterialProperty<RankFourTensor> * _Jacobian_mult_neighbor;
  bool _tangent_is_dpk1_df;
  ///@}

  /// Whether to add the directional correction term
  const bool _directional_correction;

  /// Whether to apply the volumetric locking correction to the directional correction term
  bool _volumetric_locking_correction;

  using SCZMInterfaceKernelBaseTempl<is_ad>::_base_name;
  using SCZMInterfaceKernelBaseTempl<is_ad>::_component;
  using SCZMInterfaceKernelBaseTempl<is_ad>::_ndisp;
  using SCZMInterfaceKernelBaseTempl<is_ad>::_shifted;
  using SCZMInterfaceKernelBaseTempl<is_ad>::_coord;
  using SCZMInterfaceKernelBaseTempl<is_ad>::_coord_sys;
  using SCZMInterfaceKernelBaseTempl<is_ad>::_JxW;
  using SCZMInterfaceKernelBaseTempl<is_ad>::_normals;
  using SCZMInterfaceKernelBaseTempl<is_ad>::surrogateDistance;
  using SCZMInterfaceKernelBaseTempl<is_ad>::trueNormal;
  usingGenericInterfaceKernelMembers;
};

using SCZMInterfaceKernelSmallStrain = SCZMInterfaceKernelSmallStrainTempl<false>;
using ADSCZMInterfaceKernelSmallStrain = SCZMInterfaceKernelSmallStrainTempl<true>;
