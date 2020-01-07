//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialMultiPointSource.h"

registerMooseObject("MooseTestApp", MaterialMultiPointSource);

InputParameters
MaterialMultiPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();
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
