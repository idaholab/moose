//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionDiracSource.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionDiracSource);

InputParameters
FunctionDiracSource::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addClassDescription("Residual contribution from a point source defined by a function.");
  params.addRequiredParam<FunctionName>(
      "function", "The function to use for controlling the specified dirac source.");
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point");
  return params;
}

FunctionDiracSource::FunctionDiracSource(const InputParameters & parameters)
  : DiracKernel(parameters), _function(getFunction("function")), _p(getParam<Point>("point"))
{
}

void
FunctionDiracSource::addPoints()
{
  addPoint(_p);
}

Real
FunctionDiracSource::computeQpResidual()
{
  return -_test[_i][_qp] * _function.value(_t, _p);
}
