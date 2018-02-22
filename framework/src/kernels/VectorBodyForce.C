//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorBodyForce.h"
#include "Function.h"

registerMooseObject("MooseApp", VectorBodyForce);

template <>
InputParameters
validParams<VectorBodyForce>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addClassDescription(
      "Demonstrates the multiple ways that scalar values can be introduced "
      "into kernels, e.g. (controllable) constants, functions, and "
      "postprocessors. Implements the weak form $(\\vec{\\psi_i}, -\\vec{f})$.");
  params.addParam<Real>("value", 1.0, "Coefficent to multiply by the body force term");
  params.addParam<FunctionName>(
      "function_x", "1", "A function that describes the x-component of the body force");
  params.addParam<FunctionName>(
      "function_y", "0", "A function that describes the y-component of the body force");
  params.addParam<FunctionName>(
      "function_z", "0", "A function that describes the z-component of the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  return params;
}

VectorBodyForce::VectorBodyForce(const InputParameters & parameters)
  : VectorKernel(parameters),
    _scale(getParam<Real>("value")),
    _function_x(getFunction("function_x")),
    _function_y(getFunction("function_y")),
    _function_z(getFunction("function_z")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}

Real
VectorBodyForce::computeQpResidual()
{
  Real factor = _scale * _postprocessor;
  return _test[_i][_qp] * -factor *
         RealVectorValue(_function_x.value(_t, _q_point[_qp]),
                         _function_y.value(_t, _q_point[_qp]),
                         _function_z.value(_t, _q_point[_qp]));
}
