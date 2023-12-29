//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledPressureBC.h"

registerMooseObject("TensorMechanicsApp", CoupledPressureBC);

InputParameters
CoupledPressureBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Applies a pressure from a variable on a given boundary in a given direction");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component", "component<3", "The component for the pressure");
  params.addRequiredCoupledVar("pressure", "Coupled variable containing the pressure");
  params.addParam<bool>("use_displaced_mesh", true, "Whether to use the displaced mesh.");
  return params;
}

CoupledPressureBC::CoupledPressureBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _component(getParam<unsigned int>("component")),
    _pressure(coupledValue("pressure"))
{
}

Real
CoupledPressureBC::computeQpResidual()
{
  return _pressure[_qp] * _normals[_qp](_component) * _test[_i][_qp];
}
