//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedFunctionControl.h"
#include "THMParsedFunctionWrapper.h"

registerMooseObject("ThermalHydraulicsApp", ParsedFunctionControl);

InputParameters
ParsedFunctionControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params += MooseParsedFunctionBase::validParams();
  params.addRequiredCustomTypeParam<std::string>(
      "function", "FunctionExpression", "The function to be evaluated by this control.");
  params.addClassDescription("Control that evaluates a parsed function");
  return params;
}

ParsedFunctionControl::ParsedFunctionControl(const InputParameters & parameters)
  : THMControl(parameters),
    MooseParsedFunctionBase(parameters),
    _function(verifyFunction(getParam<std::string>("function"))),
    _value(declareComponentControlData<Real>("value"))
{
}

void
ParsedFunctionControl::buildFunction()
{
  if (!_function_ptr)
  {
    THREAD_ID tid = 0;
    if (isParamValid("_tid"))
      tid = getParam<THREAD_ID>("_tid");

    _function_ptr = std::make_unique<THMParsedFunctionWrapper>(
        *_sim, _pfb_feproblem, _function, _vars, _vals, tid);
  }
}

void
ParsedFunctionControl::initialSetup()
{
  buildFunction();
}

void
ParsedFunctionControl::init()
{
  buildFunction();

  // establish dependency so that the control data graph is properly evaluated
  for (auto & ctrl_name : _function_ptr->getRealControlData())
    getControlDataByName<Real>(ctrl_name->name());
  for (auto & ctrl_name : _function_ptr->getBoolControlData())
    getControlDataByName<bool>(ctrl_name->name());
}

void
ParsedFunctionControl::execute()
{
  _value = _function_ptr->evaluate(_t, Point(0., 0., 0.));
}
