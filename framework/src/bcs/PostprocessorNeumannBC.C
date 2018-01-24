//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorNeumannBC.h"

template <>
InputParameters
validParams<PostprocessorNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<PostprocessorName>(
      "postprocessor", 0.0, "The postprocessor to use for value of the gradient on the boundary.");
  return params;
}

PostprocessorNeumannBC::PostprocessorNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _value(getPostprocessorValue("postprocessor"))
{
}

Real
PostprocessorNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _value;
}
