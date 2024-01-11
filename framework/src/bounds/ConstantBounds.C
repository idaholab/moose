//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantBounds.h"

registerMooseObject("MooseApp", ConstantBounds);
registerMooseObjectRenamed("MooseApp", ConstantBoundsAux, "06/30/2024 24:00", ConstantBounds);

InputParameters
ConstantBounds::validParams()
{
  InputParameters params = BoundsBase::validParams();
  params.addClassDescription(
      "Provides constant bound of a variable for the PETSc's variational inequalities solver");
  params.addRequiredParam<Real>("bound_value", "The value of bound for the variable");
  return params;
}

ConstantBounds::ConstantBounds(const InputParameters & parameters)
  : BoundsBase(parameters), _bound_value(getParam<Real>("bound_value"))
{
}
