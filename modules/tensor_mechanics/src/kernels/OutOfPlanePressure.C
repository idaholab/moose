/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "OutOfPlanePressure.h"
#include "Function.h"

template <>
InputParameters
validParams<OutOfPlanePressure>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("Apply pressure in the out-of-plane direction in 2D plane stress or "
                             "generalized plane strain models ");
  params.addParam<FunctionName>("function", "1.0", "Function used to prescribe pressure");
  params.addParam<PostprocessorName>("postprocessor", "Postprocessor used to prescribe pressure");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed pressure");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

OutOfPlanePressure::OutOfPlanePressure(const InputParameters & parameters)
  : Kernel(parameters),
    _postprocessor(
        parameters.isParamValid("postprocessor") ? &getPostprocessorValue("postprocessor") : NULL),
    _function(getFunction("function")),
    _factor(getParam<Real>("factor"))
{
}

Real
OutOfPlanePressure::computeQpResidual()
{
  Real val = _factor;

  val *= _function.value(_t, _q_point[_qp]);

  if (_postprocessor)
    val *= *_postprocessor;

  return val * _test[_i][_qp];
}
