//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCZMComputeLocalTractionBase.h"
#include "CZMComputeLocalTractionBase.h"

InputParameters
ADCZMComputeLocalTractionBase::validParams()
{
  InputParameters params = CZMComputeLocalTractionBase::validParams();
  return params;
}

ADCZMComputeLocalTractionBase::ADCZMComputeLocalTractionBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _interface_traction(
        declareADPropertyByName<RealVectorValue>(_base_name + "interface_traction")),
    _interface_displacement_jump(
        getADMaterialPropertyByName<RealVectorValue>(_base_name + "interface_displacement_jump"))
{
}

void
ADCZMComputeLocalTractionBase::initQpStatefulProperties()
{
  _interface_traction[_qp] = 0;
}

void
ADCZMComputeLocalTractionBase::computeQpProperties()
{
  computeInterfaceTraction();
}
