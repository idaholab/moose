//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorBodyForce.h"

registerKokkosResidualObject("MooseTestApp", KokkosVectorBodyForce);

InputParameters
KokkosVectorBodyForce::validParams()
{
  InputParameters params = Moose::Kokkos::VectorKernelValue::validParams();
  params.addClassDescription("Test vector body force with KokkosParsedFunction components.");
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addRequiredParam<FunctionName>(
      "function_x", "A KokkosParsedFunction that describes the x-component of the body force");
  params.addParam<FunctionName>(
      "function_y", "A KokkosParsedFunction that describes the y-component of the body force");
  params.addParam<FunctionName>(
      "function_z", "A KokkosParsedFunction that describes the z-component of the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  return params;
}

KokkosVectorBodyForce::KokkosVectorBodyForce(const InputParameters & parameters)
  : Moose::Kokkos::VectorKernelValue(parameters),
    _scale(getParam<Real>("value")),
    _has_function_y(isParamValid("function_y")),
    _has_function_z(isParamValid("function_z")),
    _function_x(getKokkosFunction<KokkosParsedFunction>("function_x")),
    _function_y(
        getKokkosFunction<KokkosParsedFunction>(_has_function_y ? "function_y" : "function_x")),
    _function_z(
        getKokkosFunction<KokkosParsedFunction>(_has_function_z ? "function_z" : "function_x")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}
