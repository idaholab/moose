//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNeumannBC.h"

registerMooseObject("MooseApp", ADNeumannBC);

InputParameters
ADNeumannBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC");
  params.addParam<Real>("value", 0.0, "The value of the gradient on the boundary.");
  params.declareControllable("value");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h$, "
                             "where $h$ is a constant, controllable value.");
  return params;
}

ADNeumannBC::ADNeumannBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _value(getParam<Real>("value"))
{
}

ADReal
ADNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _value;
}
