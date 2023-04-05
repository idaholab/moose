//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADAdvection.h"

registerMooseObject("NavierStokesTestApp", ADAdvection);

InputParameters
ADAdvection::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  return params;
}

ADAdvection::ADAdvection(const InputParameters & parameters)
  : ADKernel(parameters), _velocity(getADMaterialProperty<RealVectorValue>("velocity"))
{
}

ADReal
ADAdvection::computeQpResidual()
{
  return _test[_i][_qp] * _velocity[_qp] * _grad_u[_qp];
}
