//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressBase.h"

InputParameters
ComputeLagrangianStressBase::validParams()
{
  InputParameters params = Material::validParams();

  params.addParam<bool>("large_kinematics", false, "Use a large displacement stress update.");

  params.addParam<std::string>("base_name", "Material property base name");

  return params;
}

ComputeLagrangianStressBase::ComputeLagrangianStressBase(const InputParameters & parameters)
  : Material(parameters),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _cauchy_stress(declareProperty<RankTwoTensor>(_base_name + "cauchy_stress")),
    _cauchy_jacobian(declareProperty<RankFourTensor>(_base_name + "cauchy_jacobian")),
    _pk1_stress(declareProperty<RankTwoTensor>(_base_name + "pk1_stress")),
    _pk1_jacobian(declareProperty<RankFourTensor>(_base_name + "pk1_jacobian"))
{
}

void
ComputeLagrangianStressBase::initQpStatefulProperties()
{
  // Actually no need to zero out the stresses as they aren't stateful (yet)
}

void
ComputeLagrangianStressBase::computeQpProperties()
{
  computeQpStressUpdate();
}
