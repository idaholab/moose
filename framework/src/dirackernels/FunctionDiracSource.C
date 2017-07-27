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

#include "FunctionDiracSource.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionDiracSource>()
{
  InputParameters params = validParams<DiracKernel>();
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
