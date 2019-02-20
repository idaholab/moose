#include "BoolControlDataValuePostprocessor.h"
#include "Simulation.h"

registerMooseObject("THMApp", BoolControlDataValuePostprocessor);

template <>
InputParameters
validParams<BoolControlDataValuePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("control_data_name",
                                       "The name of the control data to output.");
  return params;
}

BoolControlDataValuePostprocessor::BoolControlDataValuePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _sim(*_app.parameters().getCheckedPointerParam<Simulation *>("_sim")),
    _control_data_name(getParam<std::string>("control_data_name")),
    _control_data_value(_sim.getControlData<bool>(_control_data_name)->get())
{
}

void
BoolControlDataValuePostprocessor::initialize()
{
}

void
BoolControlDataValuePostprocessor::execute()
{
}

Real
BoolControlDataValuePostprocessor::getValue()
{
  if (_control_data_value)
    return 1.;
  else
    return 0.;
}
