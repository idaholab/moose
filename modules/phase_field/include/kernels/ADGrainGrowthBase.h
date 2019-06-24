//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADAllenCahnBase.h"

#define usingGrainGrowthBaseMembers                                                                \
  usingAllenCahnBaseMembers(Real);                                                                 \
  using ADGrainGrowthBase<compute_stage>::_op_num;                                                 \
  using ADGrainGrowthBase<compute_stage>::_vals;                                                   \
  using ADGrainGrowthBase<compute_stage>::_mu

// Forward Declarations
template <ComputeStage>
class ADGrainGrowthBase;

declareADValidParams(ADGrainGrowthBase);

/**
 * This is the base class for kernels that calculate the residual for grain growth.
 * It calculates the residual of the ith order parameter, and the values of
 * all other order parameters are coupled variables and are stored in vals.
 * This is the AD equivalent of the ADGrainGrowthBase class.
 */
template <ComputeStage compute_stage>
class ADGrainGrowthBase : public ADAllenCahnBase<compute_stage, Real>
{
public:
  ADGrainGrowthBase(const InputParameters & parameters);

protected:
  const unsigned int _op_num;
  std::vector<const ADVariableValue *> _vals;
  const ADMaterialProperty(Real) & _mu;

  usingAllenCahnBaseMembers(Real);
};
