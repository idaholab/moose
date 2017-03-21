/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Gravity.h"
#include "Function.h"

template <>
InputParameters
validParams<Gravity>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Apply gravity. Value is in units of acceleration.");
  params.addParam<bool>("use_displaced_mesh", true, "Displaced mesh defaults to true");
  params.set<Real>("value") = 0.0;
  // A ConstantFunction of "1" is supplied as the default
  params.addParam<FunctionName>(
      "function", "1", "A function that describes the gravitational force");
  params.addParam<Real>("alpha", 0.0, "alpha parameter required for HHT time integration scheme");
  return params;
}

Gravity::Gravity(const InputParameters & parameters)
  : Kernel(parameters),
    _density(getMaterialProperty<Real>("density")),
    _value(getParam<Real>("value")),
    _function(getFunction("function")),
    _alpha(getParam<Real>("alpha"))
{
}

Real
Gravity::computeQpResidual()
{
  Real factor = _value * _function.value(_t + _alpha * _dt, _q_point[_qp]);
  return _density[_qp] * _test[_i][_qp] * -factor;
}
