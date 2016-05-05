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

template<>
InputParameters validParams<TimeExtremeValue>()
{
  // Define the min/max enumeration
  MooseEnum type_options("max=0 min=1", "max");

  // Define the parameters
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<MooseEnum>("value_type", type_options, "Type of extreme value to return. 'max' returns the maximum value. 'min' returns the minimum value.");
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor used for reporting time extreme values");
  params.addClassDescription("A postprocessor for reporting the max/min value of another postprocessor over time.");

  return params;
}

TimeExtremeValue::TimeExtremeValue(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _value(declareRestartableData<Real>("value"))
{
  if (!_app.isRecovering())
    _value = (_type == 0) ? -std::numeric_limits<Real>::max() : std::numeric_limits<Real>::max();
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
  }
}

Real
TimeExtremeValue::getValue()
{
  return _value;
}
