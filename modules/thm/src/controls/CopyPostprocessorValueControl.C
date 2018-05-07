#include "CopyPostprocessorValueControl.h"

registerMooseObject("RELAP7App", CopyPostprocessorValueControl);

template <>
InputParameters
validParams<CopyPostprocessorValueControl>()
{
  InputParameters params = validParams<RELAP7Control>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor.");
  return params;
}

CopyPostprocessorValueControl::CopyPostprocessorValueControl(const InputParameters & parameters)
  : RELAP7Control(parameters),
    _value(declareControlData<Real>(getParam<PostprocessorName>("postprocessor"))),
    _pps_value(getPostprocessorValue("postprocessor"))
{
}

void
CopyPostprocessorValueControl::execute()
{
  _value = _pps_value;
}
