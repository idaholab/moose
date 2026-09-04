//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "LinearFVGradientStateTest.h"

#include "LinearFVGradientReader.h"
#include "MooseLinearVariableFV.h"
#include "MooseMesh.h"
#include "SubProblem.h"

registerMooseObject("MooseTestApp", LinearFVGradientStateTest);

InputParameters
LinearFVGradientStateTest::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "Linear FV variable whose gradient is read.");
  params.addParam<unsigned int>(
      "oldest_gradient_state", 0, "Oldest gradient time state to request during construction.");
  params.addParam<unsigned int>(
      "initial_oldest_gradient_state",
      0,
      "Initial request depth used to test that a cached reader remains valid after an upgrade.");
  params.addParam<unsigned int>("state", 0, "Gradient state to read.");
  params.addParam<unsigned int>("component", 0, "Spatial gradient component to read.");
  params.addParam<MooseEnum>(
      "iteration_type", MooseEnum("time nonlinear", "time"), "Iteration type of the read state.");
  params.addParam<bool>("face", false, "Read the first internal face instead of an element.");
  params.addParam<dof_id_type>("element_id", 0, "Element ID used for element reads.");
  params.addParam<unsigned int>("late_oldest_gradient_state",
                                "Optional gradient depth to request during initial setup.");
  params.addClassDescription("Tests optional time-state storage for linear FV gradients.");
  return params;
}

LinearFVGradientStateTest::LinearFVGradientStateTest(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _variable(dynamic_cast<MooseLinearVariableFV<Real> *>(
        &_subproblem.getVariable(0, getParam<VariableName>("variable")))),
    _reader(nullptr),
    _state(getParam<unsigned int>("state")),
    _component(getParam<unsigned int>("component")),
    _iteration_type(getParam<MooseEnum>("iteration_type")),
    _face(getParam<bool>("face")),
    _element_id(getParam<dof_id_type>("element_id"))
{
  if (!_variable)
    paramError("variable", "The supplied variable must be a scalar linear FV variable.");

  _reader =
      &_variable->requestCellGradients(getParam<unsigned int>("initial_oldest_gradient_state"));
  _variable->requestCellGradients(getParam<unsigned int>("oldest_gradient_state"));
}

void
LinearFVGradientStateTest::initialSetup()
{
  GeneralPostprocessor::initialSetup();
  if (isParamValid("late_oldest_gradient_state"))
    _variable->requestCellGradients(getParam<unsigned int>("late_oldest_gradient_state"));
}

void
LinearFVGradientStateTest::execute()
{
  const auto iteration_type = _iteration_type == "time" ? Moose::SolutionIterationType::Time
                                                        : Moose::SolutionIterationType::Nonlinear;
  const Moose::StateArg state(_state, iteration_type);

  if (_face)
  {
    for (const auto & face_info : _fe_problem.mesh().faceInfo())
      if (face_info->neighborPtr())
      {
        _value = _reader->gradient(*face_info, state)(_component);
        return;
      }

    mooseError("LinearFVGradientStateTest could not find an internal face.");
  }

  _value = _reader->component(_fe_problem.mesh().elemInfo(_element_id), _component, state);
}
