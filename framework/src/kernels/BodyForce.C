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
  params.addClassDescription("Demonstrates the multiple ways that scalar values can be introduced "
                             "into kernels, e.g. (controllable) constants, functions, and "
                             "postprocessors. Implements the weak form $(\\psi_i, -f)$.");
  params.addParam<Real>("value", 1.0, "Coefficent to multiply by the body force term");
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  return params;
}

BodyForce::BodyForce(const InputParameters & parameters)
  : Kernel(parameters),
    _scale(getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}

Real
BodyForce::computeQpResidual()
{
  Real factor = _scale * _postprocessor * _function.value(_t, _q_point[_qp]);
  return _test[_i][_qp] * -factor;
}
