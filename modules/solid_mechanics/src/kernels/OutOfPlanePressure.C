/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "OutOfPlanePressure.h"
#include "Function.h"

template<>
InputParameters validParams<OutOfPlanePressure>()
{
  InputParameters params = validParams<Kernel>();

  params.addParam<FunctionName>("function","Function used to prescribe pressure");
  params.addParam<PostprocessorName>("postprocessor","Postprocessor used to prescribe pressure");
  params.addParam<Real>("factor",1.0,"Scale factor applied to prescribed pressure");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}


OutOfPlanePressure::OutOfPlanePressure(const InputParameters & parameters)
  :Kernel(parameters),
  _postprocessor(parameters.isParamValid("postprocessor") ? &getPostprocessorValue("postprocessor") : NULL),
  _function(parameters.isParamValid("function") ? &getFunction("function") : NULL),
  _factor(getParam<Real>("factor"))
{}

Real
OutOfPlanePressure::computeQpResidual()
{
  Real val = _factor;

  if (_function)
    val *= _function->value(_t,_q_point[_qp]);

  if (_postprocessor)
    val *= *_postprocessor;

  return val * _test[_i][_qp];
}
