//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeExtraStressBase.h"

InputParameters
ComputeExtraStressBase::validParams()
{
  InputParameters params = Material::validParams();
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
