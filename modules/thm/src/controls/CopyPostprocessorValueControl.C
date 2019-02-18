#include "CopyPostprocessorValueControl.h"

registerMooseObject("THMApp", CopyPostprocessorValueControl);

template <>
InputParameters
validParams<CopyPostprocessorValueControl>()
{
  InputParameters params = validParams<THMControl>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor.");
  return params;
}

CopyPostprocessorValueControl::CopyPostprocessorValueControl(const InputParameters & parameters)
  : THMControl(parameters),
    _value(declareControlData<Real>(getParam<PostprocessorName>("postprocessor"))),
    _pps_value(getPostprocessorValue("postprocessor"))
{
}

void
CopyPostprocessorValueControl::execute()
{
  _value = _pps_value;
}
