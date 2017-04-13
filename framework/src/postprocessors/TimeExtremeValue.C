/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TimeExtremeValue.h"

#include <algorithm>
#include <limits>

template <>
InputParameters
validParams<TimeExtremeValue>()
{
  // Define the min/max enumeration
  MooseEnum type_options("max=0 min=1 abs_max=2 abs_min=3", "max");

  // Define the parameters
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<MooseEnum>("value_type",
                             type_options,
                             "Type of extreme value to return."
                             "'max' returns the maximum value."
                             "'min' returns the minimum value."
                             "'abs_max' returns the maximum absolute value."
                             "'abs_min' returns the minimum absolute value.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the postprocessor used for reporting time extreme values");
  params.addClassDescription(
      "A postprocessor for reporting the extreme value of another postprocessor over time.");

  return params;
}

TimeExtremeValue::TimeExtremeValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _value(declareRestartableData<Real>("value"))
{
  if (!_app.isRecovering())
  {
    switch (_type)
    {
      case MAX:
        _value = -std::numeric_limits<Real>::max();
        break;

      case MIN:
        _value = std::numeric_limits<Real>::max();
        break;

      case ABS_MAX:
        // the max absolute value of anything is greater than or equal to 0
        _value = 0;
        break;

      case ABS_MIN:
        // the min absolute value of anything is less than this
        _value = std::numeric_limits<Real>::max();
        break;

      default:
        mooseError("Unrecognzed _type == ", _type);
    }
  }
}

void
TimeExtremeValue::execute()
{
  switch (_type)
  {
    case MAX:
      _value = std::max(_value, _postprocessor);
      break;

    case MIN:
      _value = std::min(_value, _postprocessor);
      break;

    case ABS_MAX:
      _value = std::max(_value, std::abs(_postprocessor));
      break;

    case ABS_MIN:
      _value = std::min(_value, std::abs(_postprocessor));
      break;

    default:
      mooseError("Unrecognzed _type == ", _type);
  }
}

Real
TimeExtremeValue::getValue()
{
  return _value;
}
