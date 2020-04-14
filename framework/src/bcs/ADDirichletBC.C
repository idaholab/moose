//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDirichletBC.h"

registerMooseObject("MooseApp", ADDirichletBC);

InputParameters
ADDirichletBC::validParams()
{
  InputParameters params = ADDirichletBCBase::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC");
  params.declareControllable("value");
  params.addClassDescription("Imposes the essential boundary condition $u=g$, where $g$ "
                             "is a constant, controllable value.");
  return params;
}

ADDirichletBC::ADDirichletBC(const InputParameters & parameters)
  : ADDirichletBCBase(parameters), _value(getParam<Real>("value"))
{
}

ADReal
ADDirichletBC::computeQpValue()
{
  return _value;
}
