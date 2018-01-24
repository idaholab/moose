//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplineFFn.h"

template <>
InputParameters
validParams<SplineFFn>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<FunctionName>("function", "The name of the spline function");

  return params;
}

SplineFFn::SplineFFn(const InputParameters & parameters)
  : Kernel(parameters), _fn(dynamic_cast<SplineFunction &>(getFunction("function")))
{
}

SplineFFn::~SplineFFn() {}

Real
SplineFFn::computeQpResidual()
{
  return _test[_i][_qp] * _fn.secondDerivative(_q_point[_qp]);
}
