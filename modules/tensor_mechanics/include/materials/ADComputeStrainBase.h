//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

#define usingComputeStrainBaseMembers                                                              \
  usingMaterialMembers;                                                                            \
  using ADComputeStrainBase<compute_stage>::_ndisp;                                                \
  using ADComputeStrainBase<compute_stage>::_disp;                                                 \
  using ADComputeStrainBase<compute_stage>::_grad_disp;                                            \
  using ADComputeStrainBase<compute_stage>::_base_name;                                            \
  using ADComputeStrainBase<compute_stage>::_mechanical_strain;                                    \
  using ADComputeStrainBase<compute_stage>::_global_strain;                                        \
  using ADComputeStrainBase<compute_stage>::_volumetric_locking_correction;                        \
  using ADComputeStrainBase<compute_stage>::_current_elem_volume;                                  \
  using ADComputeStrainBase<compute_stage>::_eigenstrain_names;                                    \
  using ADComputeStrainBase<compute_stage>::_eigenstrains;                                         \
  using ADComputeStrainBase<compute_stage>::_total_strain

// Forward Declarations
template <ComputeStage>
class ADComputeStrainBase;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
typedef RankTwoTensorTempl<DualReal> DualRankTwoTensor;

declareADValidParams(ADComputeStrainBase);

/**
 * ADADComputeStrainBase is the base class for strain tensors
 */
template <ComputeStage compute_stage>
class ADComputeStrainBase : public ADMaterial<compute_stage>
{
public:
  ADComputeStrainBase(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void displacementIntegrityCheck();

  /// Coupled displacement variables
  const unsigned int _ndisp;
  std::vector<const ADVariableValue *> _disp;
  std::vector<const ADVariableGradient *> _grad_disp;

  const std::string _base_name;

  ADMaterialProperty(RankTwoTensor) & _mechanical_strain;
  ADMaterialProperty(RankTwoTensor) & _total_strain;

  std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const ADMaterialProperty(RankTwoTensor) *> _eigenstrains;

  const ADMaterialProperty(RankTwoTensor) * _global_strain;

  const bool _volumetric_locking_correction;
  const Real & _current_elem_volume;

  usingMaterialMembers;
};

