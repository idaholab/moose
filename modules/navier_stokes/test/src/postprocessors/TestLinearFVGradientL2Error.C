//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "TestLinearFVGradientL2Error.h"

#include "Assembly.h"
#include "FEProblemBase.h"
#include "LinearFVGradientReader.h"
#include "MooseLinearVariableFV.h"
#include "MooseMesh.h"

#include <cmath>

registerMooseObject("NavierStokesTestApp", TestLinearFVGradientL2Error);

InputParameters
TestLinearFVGradientL2Error::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "Linear finite-volume variable to examine.");
  params.addRequiredParam<GradientMethodName>("gradient_method",
                                              "Gradient method whose field is examined.");
  params.addParam<RealVectorValue>(
      "exact_gradient", RealVectorValue(), "Constant exact gradient used for the L2 error.");
  params.addClassDescription(
      "Computes the L2 error of a named linear finite-volume gradient in test problems.");
  return params;
}

TestLinearFVGradientL2Error::TestLinearFVGradientL2Error(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _gradient(nullptr),
    _exact_gradient(getParam<RealVectorValue>("exact_gradient")),
    _error(0.0)
{
  auto * const variable = dynamic_cast<MooseLinearVariableFVReal *>(
      &_fe_problem.getVariable(_tid, getParam<VariableName>("variable")));
  if (!variable)
    paramError("variable", "The variable must be a scalar linear finite-volume variable.");

  _gradient =
      &variable->requestCellGradients(getParam<GradientMethodName>("gradient_method"));
}

void
TestLinearFVGradientL2Error::initialize()
{
  _error = 0.0;
}

void
TestLinearFVGradientL2Error::execute()
{
  for (const auto * const elem : _fe_problem.mesh().getMesh().active_local_element_ptr_range())
  {
    const auto & elem_info = _fe_problem.mesh().elemInfo(elem->id());
    const auto difference = _gradient->gradient(elem_info) - _exact_gradient;
    _error += (difference * difference) * _assembly.elementVolume(elem);
  }
}

void
TestLinearFVGradientL2Error::finalize()
{
  gatherSum(_error);
  _error = std::sqrt(_error);
}

PostprocessorValue
TestLinearFVGradientL2Error::getValue() const
{
  return _error;
}
