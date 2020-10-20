#include "ObjectiveMinimize.h"

registerMooseObject("isopodApp", ObjectiveMinimize);

InputParameters
ObjectiveMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  params.addRequiredParam<VectorPostprocessorName>(
      "subapp_vpp", "VectorPostprocessorReceiver for subapp results.");
  return params;
}

ObjectiveMinimize::ObjectiveMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _subapp_vpp_values(getVectorPostprocessorValue(
        "subapp_vpp", "temperature", false)) // fixme lynn temperature should not be hardcoded
{
}

Real
ObjectiveMinimize::computeObjective()
{
  mooseAssert(_subapp_vpp_values.size() == _measurement_vpp_values.size(),
              "measurement and subapp vpps must be the same size");

  Real val = 0;
  for (std::size_t i = 0; i < _subapp_vpp_values.size(); ++i)
  {
    Real tmp = _subapp_vpp_values[i] - _measurement_vpp_values[i];
    val += tmp * tmp;
  }

  return val;
}
