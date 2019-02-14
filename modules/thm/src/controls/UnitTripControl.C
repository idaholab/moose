#include "UnitTripControl.h"
#include "THMParsedFunctionWrapper.h"

registerMooseObject("THMApp", UnitTripControl);

template <>
InputParameters
validParams<UnitTripControl>()
{
  InputParameters params = validParams<THMControl>();
  params += validParams<MooseParsedFunctionBase>();
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
UnitTripControl::init()
{
  if (!_condition_ptr)
  {
    THREAD_ID tid = 0;
    if (isParamValid("_tid"))
      tid = getParam<THREAD_ID>("_tid");

    _condition_ptr = libmesh_make_unique<THMParsedFunctionWrapper>(
        _sim, _pfb_feproblem, _condition, _vars, _vals, tid);

    // establish dependency so that the control data graph is properly evaluated
    for (auto & ctrl_name : _condition_ptr->getRealControlData())
      getControlDataByName<Real>(ctrl_name->name());
    for (auto & ctrl_name : _condition_ptr->getBoolControlData())
      getControlDataByName<bool>(ctrl_name->name());
  }
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
