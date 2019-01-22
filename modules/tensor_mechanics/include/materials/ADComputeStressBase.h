//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTESTRESSBASE_H
#define ADCOMPUTESTRESSBASE_H

#include "ADMaterial.h"
#include "Function.h"

#define usingComputeStressBaseMembers                                                              \
  usingMaterialMembers;                                                                            \
  using ADComputeStressBase<compute_stage>::_stress;                                               \
  using ADComputeStressBase<compute_stage>::_mechanical_strain;                                    \
  using ADComputeStressBase<compute_stage>::_elastic_strain;                                       \
  using ADComputeStressBase<compute_stage>::_base_name;                                            \
  using ADComputeStressBase<compute_stage>::_elasticity_tensor;                                    \
  using ADComputeStressBase<compute_stage>::_elasticity_tensor_name;                               \
  using ADComputeStressBase<compute_stage>::_initial_stress_fcn

// Forward Declarations
template <ComputeStage>
class ADComputeStressBase;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
typedef RankTwoTensorTempl<DualReal> DualRankTwoTensor;
template <typename>
class RankFourTensorTempl;
typedef RankFourTensorTempl<Real> RankFourTensor;
typedef RankFourTensorTempl<DualReal> DualRankFourTensor;

declareADValidParams(ADComputeStressBase);

/**
 * ADComputeStressBase is the base class for stress tensors
 */
template <ComputeStage compute_stage>
class ADComputeStressBase : public ADMaterial<compute_stage>
{
public:
  ADComputeStressBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;
  virtual void computeQpStress() = 0;

  const std::string _base_name;
  const std::string _elasticity_tensor_name;

  const ADMaterialProperty(RankTwoTensor) & _mechanical_strain;
  ADMaterialProperty(RankTwoTensor) & _stress;
  ADMaterialProperty(RankTwoTensor) & _elastic_strain;

  const ADMaterialProperty(RankFourTensor) & _elasticity_tensor;

  /// initial stress components
  std::vector<Function *> _initial_stress_fcn;

  usingMaterialMembers;
};

#endif // ADCOMPUTESTRESSBASE_H
