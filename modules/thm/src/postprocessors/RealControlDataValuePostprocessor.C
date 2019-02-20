#include "RealControlDataValuePostprocessor.h"
#include "Simulation.h"

registerMooseObject("THMApp", RealControlDataValuePostprocessor);

template <>
InputParameters
validParams<RealControlDataValuePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("control_data_name",
                                       "The name of the control data to output.");
  return params;
}

RealControlDataValuePostprocessor::RealControlDataValuePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _sim(*_app.parameters().getCheckedPointerParam<Simulation *>("_sim")),
    _control_data_name(getParam<std::string>("control_data_name")),
    _control_data_value(_sim.getControlData<Real>(_control_data_name)->get())
{
}

void
RealControlDataValuePostprocessor::initialize()
{
}

void
RealControlDataValuePostprocessor::execute()
{
}

Real
RealControlDataValuePostprocessor::getValue()
{
  return _control_data_value;
}
