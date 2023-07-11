//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeComplianceTensorBase.h"
#include "Function.h"

template <bool is_ad>
InputParameters
ComputeComplianceTensorBaseTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define ");
  return params;
}

template <bool is_ad>
ComputeComplianceTensorBaseTempl<is_ad>::ComputeComplianceTensorBaseTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _compliance_tensor_name(_base_name + "compliance_tensor"),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _compliance_tensor(declareGenericProperty<RankFourTensor, is_ad>(_compliance_tensor_name)),
    _elasticity_tensor(declareGenericProperty<RankFourTensor, is_ad>(_elasticity_tensor_name))
{
}

template <bool is_ad>
void
ComputeComplianceTensorBaseTempl<is_ad>::computeQpProperties()
{
  computeQpElasticityTensor();
}


template class ComputeComplianceTensorBaseTempl<false>;
template class ComputeComplianceTensorBaseTempl<true>;
