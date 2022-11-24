//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BodyForceLM.h"

#include "Function.h"

registerMooseObject("ScalarTransportApp", BodyForceLM);

InputParameters
BodyForceLM::validParams()
{
  auto params = LMKernel::validParams();
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  params.addClassDescription(
      "Imposes a body force onto a Lagrange multiplier constrained primal equation");
  return params;
}

BodyForceLM::BodyForceLM(const InputParameters & parameters)
  : LMKernel(parameters),
    _scale(getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}

ADReal
BodyForceLM::precomputeQpResidual()
{
  return -_scale * _postprocessor * _function.value(_t, _q_point[_qp]);
}
