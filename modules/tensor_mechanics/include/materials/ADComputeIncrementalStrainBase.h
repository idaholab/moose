//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeStrainBase.h"

#define usingComputeIncrementalStrainBaseMembers                                                   \
  usingComputeStrainBaseMembers;                                                                   \
  using ADComputeIncrementalStrainBase<compute_stage>::_grad_disp_old;                             \
  using ADComputeIncrementalStrainBase<compute_stage>::_strain_rate;                               \
  using ADComputeIncrementalStrainBase<compute_stage>::_strain_increment;                          \
  using ADComputeIncrementalStrainBase<compute_stage>::_rotation_increment;                        \
  using ADComputeIncrementalStrainBase<compute_stage>::_mechanical_strain_old;                     \
  using ADComputeIncrementalStrainBase<compute_stage>::_total_strain_old;                          \
  using ADComputeIncrementalStrainBase<compute_stage>::_eigenstrains_old;                          \
  using ADComputeIncrementalStrainBase<compute_stage>::subtractEigenstrainIncrementFromStrain

template <ComputeStage>
class ADComputeIncrementalStrainBase;

declareADValidParams(ADComputeIncrementalStrainBase);

/**
 * ADComputeIncrementalStrainBase is the base class for strain tensors using incremental
 * formulations
 */
template <ComputeStage compute_stage>
class ADComputeIncrementalStrainBase : public ADComputeStrainBase<compute_stage>
{
public:
  ADComputeIncrementalStrainBase(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;

  void subtractEigenstrainIncrementFromStrain(ADRankTwoTensor & strain);

  std::vector<const VariableGradient *> _grad_disp_old;

  ADMaterialProperty(RankTwoTensor) & _strain_rate;
  ADMaterialProperty(RankTwoTensor) & _strain_increment;
  ADMaterialProperty(RankTwoTensor) & _rotation_increment;

  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;

  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains_old;

  usingComputeStrainBaseMembers;
};

