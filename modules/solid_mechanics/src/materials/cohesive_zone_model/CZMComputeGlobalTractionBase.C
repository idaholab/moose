//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMComputeGlobalTractionBase.h"

InputParameters
CZMComputeGlobalTractionBase::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();

  params.addClassDescription(
      "Base class for computing the equilibrium traction and its derivatives.");
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

CZMComputeGlobalTractionBase::CZMComputeGlobalTractionBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _traction_global(declarePropertyByName<RealVectorValue>(_base_name + "traction_global")),
    _interface_traction(
        getMaterialPropertyByName<RealVectorValue>(_base_name + "interface_traction")),
    _dtraction_djump_global(
        declarePropertyByName<RankTwoTensor>(_base_name + "dtraction_djump_global")),
    _dinterface_traction_djump(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "dinterface_traction_djump")),
    _czm_total_rotation(getMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_total_rotation"))
{
}

void
CZMComputeGlobalTractionBase::computeQpProperties()
{
  // rotate local traction and derivatives to the global coordinate system
  computeEquilibriumTracionAndDerivatives();
}
