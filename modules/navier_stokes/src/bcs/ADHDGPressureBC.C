//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes inclues
#include "NS.h"
#include "ADHDGPressureBC.h"

registerMooseObject("NavierStokesApp", ADHDGPressureBC);

InputParameters
ADHDGPressureBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredCoupledVar(NS::pressure, "The pressure");
  params.addRequiredParam<unsigned int>(
      "component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");
  return params;
}

ADHDGPressureBC::ADHDGPressureBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _pressure(adCoupledValue(NS::pressure)),
    _component(getParam<unsigned int>("component"))
{
}

ADReal
ADHDGPressureBC::computeQpResidual()
{
  return _pressure[_qp] * _normals[_qp](_component) * _test[_i][_qp];
}
