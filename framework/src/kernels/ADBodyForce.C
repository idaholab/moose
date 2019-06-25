//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBodyForce.h"

#include "Function.h"

registerADMooseObject("MooseApp", ADBodyForce);

defineADValidParams(
    ADBodyForce,
    ADKernelValue,
    params.addClassDescription(
        "Demonstrates the multiple ways that scalar values can be introduced "
        "into kernels, e.g. (controllable) constants, functions, and "
        "postprocessors. Implements the weak form $(\\psi_i, -f)$.");
    params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
    params.addParam<FunctionName>("function", "1", "A function that describes the body force");
    params.addParam<PostprocessorName>(
        "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
    params.declareControllable("value"););

template <ComputeStage compute_stage>
ADBodyForce<compute_stage>::ADBodyForce(const InputParameters & parameters)
  : ADKernelValue<compute_stage>(parameters),
    _scale(getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}

template <ComputeStage compute_stage>
ADReal
ADBodyForce<compute_stage>::precomputeQpResidual()
{
  return -_scale * _postprocessor * _function.value(_t, _q_point[_qp]);
}
