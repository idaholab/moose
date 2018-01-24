//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirichletBC.h"

template <>
InputParameters
validParams<DirichletBC>()
{
  InputParameters p = validParams<NodalBC>();
  p.addRequiredParam<Real>("value", "Value of the BC");
  p.declareControllable("value");
  p.addClassDescription("Imposes the essential boundary condition $u=g$, where $g$ "
                        "is a constant, controllable value.");
  return p;
}

DirichletBC::DirichletBC(const InputParameters & parameters)
  : NodalBC(parameters), _value(getParam<Real>("value"))
{
}

Real
DirichletBC::computeQpResidual()
{
  return _u[_qp] - _value;
}
