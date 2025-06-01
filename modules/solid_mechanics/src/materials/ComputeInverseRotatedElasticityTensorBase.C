//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeInverseRotatedElasticityTensorBase.h"
#include "RotationTensor.h"

template <bool is_ad>
InputParameters
ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>::validParams()
{
  InputParameters params = ComputeComplianceTensorBaseTempl<is_ad>::validParams();
  return params;
}

template <bool is_ad>
ComputeInverseRotatedElasticityTensorBaseTempl<
    is_ad>::ComputeInverseRotatedElasticityTensorBaseTempl(const InputParameters & parameters)
  : ComputeComplianceTensorBaseTempl<is_ad>(parameters)
{
}

template class ComputeInverseRotatedElasticityTensorBaseTempl<false>;
template class ComputeInverseRotatedElasticityTensorBaseTempl<true>;
