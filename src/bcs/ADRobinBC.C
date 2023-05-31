//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRobinBC.h"

registerMooseObject("troutApp", ADRobinBC);

InputParameters
ADRobinBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription("Imposes robin the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=u$.");
  params.addParam<Real>("coef", 1.0, "Coefficent multiplier for the robin bc term.");
  return params;
}

ADRobinBC::ADRobinBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _coef(getParam<Real>("coef"))
{
}

ADReal
ADRobinBC::computeQpResidual()
{
  return _coef * _test[_i][_qp] * _u[_qp];
}
