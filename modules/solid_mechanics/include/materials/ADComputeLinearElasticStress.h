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

/**
 * ADComputeLinearElasticStress computes the stress following linear elasticity theory (small
 * strains)
 */
template <typename R2, typename R4>
class ADComputeLinearElasticStressTempl : public ADComputeStressBaseTempl<R2>
{
public:
  static InputParameters validParams();

  ADComputeLinearElasticStressTempl(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual void computeQpStress() override;

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;

  /// Elasticity tensor material property
  const ADMaterialProperty<R4> & _elasticity_tensor;

  usingComputeStressBaseMembers;
};

typedef ADComputeLinearElasticStressTempl<RankTwoTensor, RankFourTensor>
    ADComputeLinearElasticStress;
typedef ADComputeLinearElasticStressTempl<SymmetricRankTwoTensor, SymmetricRankFourTensor>
    ADSymmetricLinearElasticStress;
