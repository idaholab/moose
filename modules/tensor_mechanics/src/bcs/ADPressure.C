//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPressure.h"
#include "Function.h"
#include "MooseError.h"

registerADMooseObject("TensorMechanicsApp", ADPressure);

defineADValidParams(
    ADPressure,
    ADIntegratedBC,
    params.addClassDescription("Applies a pressure on a given boundary in a given direction");
    params.addRequiredRangeCheckedParam<unsigned int>("component",
                                                      "component <= 2",
                                                      "The component for the pressure");
    params.addParam<Real>("constant", 1.0, "The magnitude to use in computing the pressure");
    params.addParam<FunctionName>("function", "The function that describes the pressure");
    params.addParam<PostprocessorName>("postprocessor",
                                       "Postprocessor that will supply the pressure value");
    params.addParam<Real>("alpha", 0.0, "alpha parameter required for HHT time integration scheme");
    params.set<bool>("use_displaced_mesh") = true;);

template <ComputeStage compute_stage>
ADPressure<compute_stage>::ADPressure(const InputParameters & parameters)
  : ADIntegratedBC<compute_stage>(parameters),
    _component(getParam<unsigned int>("component")),
    _constant(getParam<Real>("constant")),
    _function(isParamValid("function") ? &this->getFunction("function") : nullptr),
    _postprocessor(isParamValid("postprocessor") ? &this->getPostprocessorValue("postprocessor")
                                                 : nullptr),
    _alpha(getParam<Real>("alpha"))
{
}

template <ComputeStage compute_stage>
ADReal
ADPressure<compute_stage>::computeQpResidual()
{
  ADReal factor = _constant;

  if (_function)
    factor *= _function->value(_t + _alpha * _dt, _q_point[_qp]);

  if (_postprocessor)
    factor *= *_postprocessor;

  return factor * (_normals[_qp](_component) * _test[_i][_qp]);
}
