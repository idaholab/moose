#include "RealComponentParameterValuePostprocessor.h"
#include "ControllableParameter.h"
#include "InputParameterWarehouse.h"

registerMooseObject("THMApp", RealComponentParameterValuePostprocessor);

template <>
InputParameters
validParams<RealComponentParameterValuePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("component", "The name of the component to be controlled.");
  params.addRequiredParam<std::string>(
      "parameter", "The name of the parameter in the component to be controlled.");
  return params;
}

RealComponentParameterValuePostprocessor::RealComponentParameterValuePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _input_parameter_warehouse(_app.getInputParameterWarehouse()),
    _component_name(getParam<std::string>("component")),
    _param_name(getParam<std::string>("parameter")),
    _ctrl_param_name("component/" + _component_name + "/" + _param_name)
{
}

void
RealComponentParameterValuePostprocessor::initialize()
{
}

void
RealComponentParameterValuePostprocessor::execute()
{
  std::vector<Real> values =
      _input_parameter_warehouse.getControllableParameterValues<Real>(_ctrl_param_name);

  _value = values[0];
}

Real
RealComponentParameterValuePostprocessor::getValue()
{
  return _value;
}
