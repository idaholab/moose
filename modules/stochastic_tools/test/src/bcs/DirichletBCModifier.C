//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirichletBCModifier.h"

registerMooseObject("StochasticToolsTestApp", DirichletBCModifier);

InputParameters
DirichletBCModifier::validParams()
{
  InputParameters params = DirichletBCBase::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC.");
  params.declareControllable("value");
  params.addClassDescription("Subtracts a constant value from the residual at the "
                             "boundary nodes. It can be used as a modifier for the "
                             "DirichletBC, but cannot be used alone.");
  return params;
}

DirichletBCModifier::DirichletBCModifier(const InputParameters & parameters)
  : DirichletBCBase(parameters), _value(getParam<Real>("value"))
{
}

Real
DirichletBCModifier::computeQpValue()
{
  // Since the definition in DirichletBCBase is _u[qp] - computeQpValue() and
  // we want - _value only.
  return _u[_qp] + _value;
}
