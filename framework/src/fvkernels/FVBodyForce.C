//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVBodyForce.h"
#include "Function.h"

registerMooseObject("MooseApp", FVBodyForce);

InputParameters
FVBodyForce::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Demonstrates the multiple ways that scalar values can be introduced "
      "into finite volume kernels, e.g. (controllable) constants, functions, and "
      "postprocessors.");
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  return params;
}

FVBodyForce::FVBodyForce(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _scale(getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}

ADReal
FVBodyForce::computeQpResidual()
{
  // FVKernels are not evaluated at quadrature points like our finite element kernels.
  // computeQpResidual is currrently only called once per element. With that in mind we'll just read
  // our function at the element centroid
  Real factor = _scale * _postprocessor * _function.value(_t, _current_elem->vertex_average());
  return -factor;
}
