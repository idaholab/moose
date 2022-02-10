//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseFunction.h"

registerMooseObject("ThermalHydraulicsApp", PiecewiseFunction);

InputParameters
PiecewiseFunction::validParams()
{
  InputParameters params = Function::validParams();

  MooseEnum axis("x=0 y=1 z=2 t=3");
  params.addRequiredParam<MooseEnum>("axis", axis, "Axis on which the N-1 delimiting points lie");
  params.addRequiredParam<std::vector<Real>>(
      "axis_coordinates",
      "N-1 coordinates in the chosen axis, in increasing order, delimiting the N function regions");
  params.addRequiredParam<std::vector<FunctionName>>("functions", "Functions in the N regions");

  params.addClassDescription(
      "Function which provides a piecewise representation of arbitrary functions");

  return params;
}

PiecewiseFunction::PiecewiseFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _component(getParam<MooseEnum>("axis")),
    _use_time(_component == 3),
    _axis_coordinates(getParam<std::vector<Real>>("axis_coordinates")),

    _function_names(getParam<std::vector<FunctionName>>("functions")),
    _n_functions(_function_names.size()),
    _functions(_n_functions)
{
  // Check that number of points is consistent with number of functions.
  if (_axis_coordinates.size() != _n_functions - 1)
    mooseError(name(),
               ": The number of entries in the parameter 'axis_coordinates' must"
               " equal the number of entries in the parameter 'functions' minus one.");

  // Check that coordinates are in ascending order.
  Real previous_coordinate = 0;
  for (unsigned int i = 0; i < _axis_coordinates.size(); i++)
  {
    if (i != 0 && _axis_coordinates[i] < previous_coordinate)
      mooseError(name(),
                 ": The entries in the parameter 'axis_coordinates' must be in ascending order.");
    previous_coordinate = _axis_coordinates[i];
  }

  // Store functions and check to make sure there is no self-reference.
  for (unsigned int i = 0; i < _n_functions; i++)
  {
    if (_function_names[i] == name())
      mooseError(name(), ": This function cannot use its own name in the 'functions' parameter.");

    _functions[i] = &getFunctionByName(_function_names[i]);
  }
}

unsigned int
PiecewiseFunction::getFunctionIndex(Real t, const Point & p) const
{
  const Real x = _use_time ? t : p(_component);

  // Check if position is in the first N-1 regions.
  for (unsigned int i = 0; i < _n_functions - 1; i++)
    if (x < _axis_coordinates[i])
      return i;

  // If function has not yet returned, it must be in the last region.
  return _n_functions - 1;
}

Real
PiecewiseFunction::value(Real t, const Point & p) const
{
  const unsigned int i = getFunctionIndex(t, p);
  return _functions[i]->value(t, p);
}

RealVectorValue
PiecewiseFunction::gradient(Real t, const Point & p) const
{
  const unsigned int i = getFunctionIndex(t, p);
  return _functions[i]->gradient(t, p);
}

Real
PiecewiseFunction::timeDerivative(Real t, const Point & p) const
{
  const unsigned int i = getFunctionIndex(t, p);
  return _functions[i]->timeDerivative(t, p);
}
