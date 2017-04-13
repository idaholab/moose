/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeExtraStressBase.h"

template <>
InputParameters
validParams<ComputeExtraStressBase>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  return params;
}

ComputeExtraStressBase::ComputeExtraStressBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _extra_stress_name(_base_name + "extra_stress"),
    _extra_stress(declareProperty<RankTwoTensor>(_base_name + "extra_stress"))
{
}

void
ComputeExtraStressBase::computeQpProperties()
{
  computeQpExtraStress();
}
