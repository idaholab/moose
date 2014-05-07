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

#include "BodyForce.h"

// MOOSE
#include "Function.h"

template<>
InputParameters validParams<BodyForce>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("value")=0.0;
  // A ConstantFunction of "1" is supplied as the default
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  return params;
}

BodyForce::BodyForce(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _value(getParam<Real>("value")),
    _function(getFunction("function"))
{
}

Real
BodyForce::computeQpResidual()
{
  Real factor = _value * _function.value(_t, _q_point[_qp]);
  return _test[_i][_qp] * -factor;
}
