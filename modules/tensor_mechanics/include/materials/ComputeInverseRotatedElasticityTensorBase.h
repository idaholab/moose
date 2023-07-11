//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeComplianceTensorBase.h"
#include "RankTwoTensor.h"

template <bool is_ad>
class ComputeInverseRotatedElasticityTensorBaseTempl : public ComputeComplianceTensorBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeInverseRotatedElasticityTensorBaseTempl(const InputParameters & parameters);

protected:
};

typedef ComputeInverseRotatedElasticityTensorBaseTempl<false> ComputeInverseRotatedElasticityTensorBase;
typedef ComputeInverseRotatedElasticityTensorBaseTempl<true> ADComputeInverseRotatedElasticityTensorBase;
