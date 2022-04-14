//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UnitTripControl.h"
#include "THMParsedFunctionWrapper.h"

registerMooseObject("ThermalHydraulicsApp", UnitTripControl);

InputParameters
UnitTripControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params += MooseParsedFunctionBase::validParams();
  params.addRequiredCustomTypeParam<std::string>(
      "condition",
      "FunctionExpression",
      "The condition that this trip unit uses to determine its state.");
  params.addParam<bool>("latch",
                        false,
                        "Determines if the output of this control stays true after the trip went "
                        "from false to true.");
  return params;
}

UnitTripControl::UnitTripControl(const InputParameters & parameters)
  : THMControl(parameters),
    MooseParsedFunctionBase(parameters),
    _condition(verifyFunction(getParam<std::string>("condition"))),
    _state(declareComponentControlData<bool>("state")),
    _latch(getParam<bool>("latch")),
    _tripped(false)
{
}

void
UnitTripControl::buildConditionFunction()
{
  if (!_condition_ptr)
  {
    THREAD_ID tid = 0;
    if (isParamValid("_tid"))
      tid = getParam<THREAD_ID>("_tid");

    _condition_ptr = std::make_unique<THMParsedFunctionWrapper>(
        *_sim, _pfb_feproblem, _condition, _vars, _vals, tid);
  }
}

void
UnitTripControl::initialSetup()
{
  buildConditionFunction();
}

void
UnitTripControl::init()
{
  buildConditionFunction();

  // establish dependency so that the control data graph is properly evaluated
  for (auto & ctrl_name : _condition_ptr->getRealControlData())
    getControlDataByName<Real>(ctrl_name->name());
  for (auto & ctrl_name : _condition_ptr->getBoolControlData())
    getControlDataByName<bool>(ctrl_name->name());
}

void
UnitTripControl::execute()
{
  if (_latch && _tripped)
  {
    _state = true;
    return;
  }

  Real result = _condition_ptr->evaluate(_t, Point(0., 0., 0.));
  if (result == 0.)
    _state = false;
  else if (result == 1.)
  {
    _state = true;
    _tripped = true;
  }
  else
    mooseError(name(),
               ": The user-provided condition expression did not return a boolean value (0 or 1).");
}
