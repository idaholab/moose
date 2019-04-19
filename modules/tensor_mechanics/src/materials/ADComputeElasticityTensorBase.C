//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeElasticityTensorBase.h"
#include "Function.h"

defineADValidParams(ADComputeElasticityTensorBase,
                    ADMaterial,
                    params.addParam<FunctionName>(
                        "elasticity_tensor_prefactor",
                        "Optional function to use as a scalar prefactor on the elasticity tensor.");
                    params.addParam<std::string>(
                        "base_name",
                        "Optional parameter that allows the user to define multiple mechanics "
                        "material systems on the same block, i.e. for multiple phases"););

template <ComputeStage compute_stage>
ADComputeElasticityTensorBase<compute_stage>::ADComputeElasticityTensorBase(
    const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    GuaranteeProvider(this),
    _base_name(isParamValid("base_name") ? adGetParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(adDeclareADProperty<RankFourTensor>(_elasticity_tensor_name)),
    _prefactor_function(isParamValid("elasticity_tensor_prefactor")
                            ? &getFunction("elasticity_tensor_prefactor")
                            : nullptr)
{
}

template <ComputeStage compute_stage>
void
ADComputeElasticityTensorBase<compute_stage>::computeQpProperties()
{
  computeQpElasticityTensor();

  // Multiply by prefactor
  if (_prefactor_function)
    _elasticity_tensor[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);
}

// explicit instantiation is required for AD base classes
adBaseClass(ADComputeElasticityTensorBase);
