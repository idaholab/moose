//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeExtremeValue.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", TimeExtremeValue);

InputParameters
TimeExtremeValue::validParams()
{
  // Define the min/max enumeration
  MooseEnum type_options("max=0 min=1 abs_max=2 abs_min=3", "max");

  // Define the extreme value/time enumeration
  MooseEnum output_type_options("extreme_value=0 time=1", "extreme_value");

  // Define the parameters
  InputParameters params = GeneralPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type",
                             type_options,
                             "Type of extreme value to return."
                             "'max' returns the maximum value."
                             "'min' returns the minimum value."
                             "'abs_max' returns the maximum absolute value."
                             "'abs_min' returns the minimum absolute value.");
  params.addParam<MooseEnum>("output_type",
                             output_type_options,
                             "Output to return. 'extreme_value' returns the extreme_value. 'time' "
                             "returns the time at which the extreme_value occurred.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the postprocessor used for reporting time extreme values");
  params.addClassDescription(
      "A postprocessor for reporting the extreme value of another postprocessor over time.");

  return params;
}

TimeExtremeValue::TimeExtremeValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _type(getParam<MooseEnum>("value_type").getEnum<ExtremeType>()),
    _output_type(getParam<MooseEnum>("output_type").getEnum<OutputType>()),
    _value(declareRestartableData<Real>("value")),
    _time(declareRestartableData<Real>("time"))
{
  if (!_app.isRecovering())
  {
    switch (_type)
    {
      case ExtremeType::MAX:
        _value = -std::numeric_limits<Real>::max();
        break;

      case ExtremeType::MIN:
        _value = std::numeric_limits<Real>::max();
        break;

      case ExtremeType::ABS_MAX:
        // the max absolute value of anything is greater than or equal to 0
        _value = 0;
        break;

      case ExtremeType::ABS_MIN:
        // the min absolute value of anything is less than this
        _value = std::numeric_limits<Real>::max();
        break;

      default:
        mooseError("Unrecognzed ExtremeType");

        _time = 0; // Time has to be greater than or equal to zero.
    }
  }
}

void
TimeExtremeValue::execute()
{
  switch (_type)
  {
    case ExtremeType::MAX:
      _value = std::max(_value, _postprocessor);
      _time = (_value == _postprocessor) ? _t : _time;
      break;

    case ExtremeType::MIN:
      _value = std::min(_value, _postprocessor);
      _time = (_value == _postprocessor) ? _t : _time;
      break;

    case ExtremeType::ABS_MAX:
      _value = std::max(_value, std::abs(_postprocessor));
      _time = (_value == std::abs(_postprocessor)) ? _t : _time;
      break;

    case ExtremeType::ABS_MIN:
      _value = std::min(_value, std::abs(_postprocessor));
      _time = (_value == std::abs(_postprocessor)) ? _t : _time;
      break;

    default:
      mooseError("Unrecognized ExtremeType");
  }
}

Real
TimeExtremeValue::getValue()
{
  if (_output_type == OutputType::EXTREME_VALUE)
    return _value;
  else
    return _time;
}
