/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ExampleDirac.h"

template <>
InputParameters
validParams<ExampleDirac>()
{
  InputParameters params = validParams<DiracKernel>();
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
