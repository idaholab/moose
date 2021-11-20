//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleDirac.h"

registerMooseObject("ExampleApp", ExampleDirac);

InputParameters
ExampleDirac::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addRequiredParam<Real>("value", "The value of the point source");
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point");
  return params;
}

ExampleDirac::ExampleDirac(const InputParameters & parameters)
  : DiracKernel(parameters), _value(getParam<Real>("value")), _point(getParam<Point>("point"))
{
}

void
ExampleDirac::addPoints()
{
  // Add a point from the input file
  addPoint(_point);

  // Add another point not read from the input file
  addPoint(Point(4.9, 0.9, 0.9));
}

Real
ExampleDirac::computeQpResidual()
{
  // This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * _value;
}
