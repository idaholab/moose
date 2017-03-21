/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "BodyForceVoid.h"

// MOOSE
#include "Function.h"

template <>
InputParameters
validParams<BodyForceVoid>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("value") = 0.0;
  params.addRequiredCoupledVar("c", "void concentration");
  params.addParam<FunctionName>("function", "", "A function that describes the body force");
  return params;
}

BodyForceVoid::BodyForceVoid(const InputParameters & parameters)
  : Kernel(parameters),
    _value(getParam<Real>("value")),
    _c(coupledValue("c")),
    _has_function(getParam<FunctionName>("function") != ""),
    _function(_has_function ? &getFunction("function") : NULL)
{
}

Real
BodyForceVoid::computeQpResidual()
{
  Real factor = _value;
  if (_has_function)
  {
    factor *= _function->value(_t, _q_point[_qp]);
  }
  return _test[_i][_qp] * -factor * (1 - _c[_qp] * _c[_qp]);
}
