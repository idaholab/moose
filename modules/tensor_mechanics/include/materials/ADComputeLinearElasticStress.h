//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeStressBase.h"

#define usingComputeLinearElasticStressMembers                                                     \
  usingComputeStressBaseMembers;                                                                   \
  using ADComputeLinearElasticStress<compute_stage>::_elasticity_tensor;                           \
  using ADComputeLinearElasticStress<compute_stage>::_elasticity_tensor_name;

template <ComputeStage>
class ADComputeLinearElasticStress;

declareADValidParams(ADComputeLinearElasticStress);

/**
 * ADComputeLinearElasticStress computes the stress following linear elasticity theory (small
 * strains)
 */
template <ComputeStage compute_stage>
class ADComputeLinearElasticStress : public ADComputeStressBase<compute_stage>
{
public:
  ADComputeLinearElasticStress(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  virtual void computeQpStress() override;

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const ADMaterialProperty(RankFourTensor) & _elasticity_tensor;

  usingComputeStressBaseMembers;
};

