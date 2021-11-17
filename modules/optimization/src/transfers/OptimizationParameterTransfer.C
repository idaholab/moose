#include "OptimizationParameterTransfer.h"
#include "MultiApp.h"
#include "ControlsReceiver.h"
#include "InputParameterWarehouse.h"

registerMooseObject("isopodApp", OptimizationParameterTransfer);

InputParameters
OptimizationParameterTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription("Copies optimization data to a ControlsReceiver object.");

  params.addRequiredParam<std::vector<ReporterValueName>>(
      "value_names", "Name of parameter values from OptimizationReporter.");
  params.addRequiredParam<std::vector<std::string>>(
      "parameters",
      "A list of parameters (on the sub application) to control with values from "
      "'parameter_names'.");

  params.addRequiredParam<std::string>("to_control",
                                       "The name of the 'ControlsReceiver' on the sub application "
                                       "to which the optimization parameters will be transferred.");
  params.set<MultiMooseEnum>("direction") = "to_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");

  return params;
}

OptimizationParameterTransfer::OptimizationParameterTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    ReporterInterface(this),
    _value_names(getParam<std::vector<ReporterValueName>>("value_names")),
    _parameters(getParam<std::vector<std::string>>("parameters")),
    _receiver_name(getParam<std::string>("to_control"))
{
  if (_value_names.size() != _parameters.size())
    paramError("parameters", "Number of 'value_names' must match number of 'parameters'.");
}

void
OptimizationParameterTransfer::initialSetup()
{
  _values.reserve(_value_names.size());
  for (const auto & name : _value_names)
  {
    ReporterName rname("OptimizationReporter", name);
    _values.push_back(&getReporterValueByName<std::vector<Real>>(rname, REPORTER_MODE_REPLICATED));
  }
}

void
OptimizationParameterTransfer::execute()
{
  std::vector<Real> values_full;
  for (const auto & v : _values)
    values_full.insert(values_full.end(), v->begin(), v->end());

  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
    {
      ControlsReceiver * ptr = getReceiver(i);
      ptr->transfer(_parameters, values_full);
    }
}

ControlsReceiver *
OptimizationParameterTransfer::getReceiver(unsigned int app_index)
{
  // Test that the sub-application has the given Control object
  FEProblemBase & to_problem = _multi_app->appProblemBase(app_index);
  ExecuteMooseObjectWarehouse<Control> & control_wh = to_problem.getControlWarehouse();
  if (!control_wh.hasActiveObject(_receiver_name))
    mooseError("The sub-application (",
               _multi_app->name(),
               ") does not contain a Control object with the name '",
               _receiver_name,
               "'.");

  ControlsReceiver * ptr =
      dynamic_cast<ControlsReceiver *>(control_wh.getActiveObject(_receiver_name).get());

  if (!ptr)
    mooseError(
        "The sub-application (",
        _multi_app->name(),
        ") Control object for the 'to_control' parameter must be of type 'ControlsReceiver'.");

  return ptr;
}
