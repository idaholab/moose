//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADGravity.h"

#include "Function.h"

registerADMooseObject("TensorMechanicsApp", ADGravity);

/**
 * This macro defines the valid parameters for
 * this Kernel and their default values
 */
defineADValidParams(
    ADGravity,
    ADKernelValue,
    params.addClassDescription("Apply gravity. Value is in units of acceleration.");
    params.addParam<bool>("use_displaced_mesh", true, "Displaced mesh defaults to true");
    params.set<Real>("value") = 0.0;
    params.addParam<FunctionName>("function",
                                  "1",
                                  "A function that describes the gravitational force");
    params.addParam<Real>("alpha",
                          0.0,
                          "alpha parameter required for HHT time integration scheme"););

template <ComputeStage compute_stage>
ADGravity<compute_stage>::ADGravity(const InputParameters & parameters)
  : ADKernelValue<compute_stage>(parameters),
    _density(adGetADMaterialProperty<Real>("density")),
    _value(adGetParam<Real>("value")),
    _function(getFunction("function")),
    _alpha(adGetParam<Real>("alpha"))
{
}

template <ComputeStage compute_stage>
ADResidual
ADGravity<compute_stage>::precomputeQpResidual()
{
  Real factor = _value * _function.value(_t + _alpha * _dt, _q_point[_qp]);
  return -_density[_qp] * factor;
}
