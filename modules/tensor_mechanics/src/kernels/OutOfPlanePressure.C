//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OutOfPlanePressure.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", OutOfPlanePressure);

InputParameters
OutOfPlanePressure::validParams()
{
  InputParameters params = Kernel::validParams();

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
