//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfacialSource.h"

// MOOSE
#include "Function.h"

registerMooseObject("MooseTestApp", InterfacialSource);

defineLegacyParams(InterfacialSource);

InputParameters
InterfacialSource::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription(
      "Demonstrates the multiple ways that scalar values can be introduced "
      "into interface kernels, e.g. (controllable) constants, functions, and "
      "postprocessors.");
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  return params;
}

InterfacialSource::InterfacialSource(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _scale(getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}

Real
InterfacialSource::computeQpResidual(Moose::DGResidualType type)
{
  Real residual = -_scale * _postprocessor * _function.value(_t, _q_point[_qp]);

  switch (type)
  {
    case Moose::Element:
      residual *= _test[_i][_qp];
      break;

    case Moose::Neighbor:
      residual *= _test_neighbor[_i][_qp];
      break;

    default:
      mooseError("Unrecognized Moose::DGResidualType type");
  }

  return residual;
}
