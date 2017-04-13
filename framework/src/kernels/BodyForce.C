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

template <>
InputParameters
validParams<BodyForce>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("value", 1.0, "Coefficent to multiply by the body force term");
  // A ConstantFunction of "1" is supplied as the default
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addParam<PostprocessorName>("postprocessor",
                                     "A postprocessor whose value is multiplied by the body force");
  return params;
}

BodyForce::BodyForce(const InputParameters & parameters)
  : Kernel(parameters),
    _value(getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(
        parameters.isParamValid("postprocessor") ? &getPostprocessorValue("postprocessor") : NULL)
{
}

Real
BodyForce::computeQpResidual()
{
  Real factor = _value * _function.value(_t, _q_point[_qp]);
  if (_postprocessor)
    factor *= *_postprocessor;
  return _test[_i][_qp] * -factor;
}
