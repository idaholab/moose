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

#include "MaterialMultiPointSource.h"

template <>
InputParameters
validParams<MaterialMultiPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<std::vector<Point>>("points", "The x,y,z coordinates of the points");
  return params;
}

MaterialMultiPointSource::MaterialMultiPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    _points(getParam<std::vector<Point>>("points")),
    _value(getMaterialProperty<Real>("matp"))
{
}

void
MaterialMultiPointSource::addPoints()
{
  for (unsigned int i = 0; i < _points.size(); ++i)
    addPoint(_points[i]);
}

Real
MaterialMultiPointSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been
  // brought over to the left side.
  return -_test[_i][_qp] * _value[_qp];
}
