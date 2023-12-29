//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCZMComputeGlobalTractionBase.h"

InputParameters
ADCZMComputeGlobalTractionBase::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

ADCZMComputeGlobalTractionBase::ADCZMComputeGlobalTractionBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _traction_global(declareADPropertyByName<RealVectorValue>(_base_name + "traction_global")),
    _interface_traction(
        getADMaterialPropertyByName<RealVectorValue>(_base_name + "interface_traction")),
    _czm_total_rotation(
        getADMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_total_rotation"))
{
}

void
ADCZMComputeGlobalTractionBase::computeQpProperties()
{
  computeEquilibriumTracion();
}
