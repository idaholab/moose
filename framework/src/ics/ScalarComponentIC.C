//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarComponentIC.h"
#include "MooseVariableScalar.h"

registerMooseObject("MooseApp", ScalarComponentIC);

InputParameters
ScalarComponentIC::validParams()
{
  InputParameters params = ScalarInitialCondition::validParams();
  params.addClassDescription(
      "Initial condition to set different values on each component of scalar variable.");
  params.addRequiredParam<std::vector<Real>>("values",
                                             "Initial values to initialize the scalar variable.");
  return params;
}

ScalarComponentIC::ScalarComponentIC(const InputParameters & parameters)
  : ScalarInitialCondition(parameters), _initial_values(getParam<std::vector<Real>>("values"))
{
  if (_initial_values.size() < _var.order())
    mooseError("The initial vector values size given to the scalar variable '",
               name(),
               "' has wrong size.");
}

Real
ScalarComponentIC::value()
{
  return _initial_values[_i];
}
