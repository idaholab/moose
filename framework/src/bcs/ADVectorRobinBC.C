//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorRobinBC.h"

registerMooseObject("MooseApp", ADVectorRobinBC);

InputParameters
ADVectorRobinBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addClassDescription("Imposes the Robin integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=u$.");
  params.addParam<Real>(
      "coefficient", 1.0, "Coefficient multiplier for the Robin boundary condition term.");
  return params;
}

ADVectorRobinBC::ADVectorRobinBC(const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters), _coef(getParam<Real>("coefficient"))
{
}

ADReal
ADVectorRobinBC::computeQpResidual()
{
  return _coef * _test[_i][_qp] * _u[_qp];
}
