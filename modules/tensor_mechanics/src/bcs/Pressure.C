//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Pressure.h"
#include "Function.h"
#include "MooseError.h"

template <>
InputParameters
validParams<Pressure>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addClassDescription("Applies a pressure on a given boundary in a given direction");
  params.addRequiredParam<unsigned int>("component", "The component for the pressure");
  params.addParam<Real>("factor", 1.0, "The magnitude to use in computing the pressure");
  params.addParam<FunctionName>("function", "The function that describes the pressure");
  params.addParam<PostprocessorName>("postprocessor",
                                     "Postprocessor that will supply the pressure value");
  params.addParam<Real>("alpha", 0.0, "alpha parameter required for HHT time integration scheme");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

Pressure::Pressure(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _component(getParam<unsigned int>("component")),
    _factor(getParam<Real>("factor")),
    _function(isParamValid("function") ? &getFunction("function") : NULL),
    _postprocessor(isParamValid("postprocessor") ? &getPostprocessorValue("postprocessor") : NULL),
    _alpha(getParam<Real>("alpha"))
{
  if (_component > 2)
    mooseError("Invalid component given for ", name(), ": ", _component, ".\n");
}

Real
Pressure::computeQpResidual()
{
  Real factor = _factor;

  if (_function)
    factor *= _function->value(_t + _alpha * _dt, _q_point[_qp]);

  if (_postprocessor)
    factor *= *_postprocessor;

  return factor * (_normals[_qp](_component) * _test[_i][_qp]);
}
