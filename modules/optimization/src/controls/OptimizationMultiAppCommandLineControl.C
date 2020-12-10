// MOOSE includes
#include "OptimizationMultiAppCommandLineControl.h"
#include "MultiApp.h"

registerMooseObject("StochasticToolsApp", OptimizationMultiAppCommandLineControl);

InputParameters
OptimizationMultiAppCommandLineControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription("Control for modifying the command line arguments of MultiApps from "
                             "optimization parameters.");

  // Set and suppress the 'execute_on' flag, it doesn't work with any other flag
  params.set<ExecFlagEnum>("execute_on") = {EXEC_PRE_MULTIAPP_SETUP};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  params.addRequiredParam<MultiAppName>("multi_app", "The name of the MultiApp to control.");

  params.addRequiredParam<std::vector<ReporterValueName>>(
      "value_names", "Name of parameter values from FormFunction.");
  params.addRequiredParam<std::vector<std::string>>(
      "parameters",
      "A list of parameters (on the sub application) to control with values from "
      "'parameter_names'.");

  return params;
}

OptimizationMultiAppCommandLineControl::OptimizationMultiAppCommandLineControl(
    const InputParameters & parameters)
  : Control(parameters),
    ReporterInterface(this),
    _multi_app(_fe_problem.getMultiApp(getParam<MultiAppName>("multi_app"))),
    _value_names(getParam<std::vector<ReporterValueName>>("value_names")),
    _parameters(getParam<std::vector<std::string>>("parameters"))
{
  if (!_multi_app->isParamValid("reset_app"))
    paramError("multi_app", "Can only use this control with OptimizeFullSolveMultiApp.");
  else if (!_multi_app->getParam<bool>("reset_app"))
    paramError("multi_app", "MultiApp block must have 'reset_app = true'.");

  if (_value_names.size() != _parameters.size())
    paramError("parameters", "Number of 'value_names' must match number of 'parameters'.");
}

void
OptimizationMultiAppCommandLineControl::initialSetup()
{
  // Do not put anything here, this method is being called after execute because the execute_on
  // is set to PRE_MULTIAPP_SETUP for this class. It won't work any other way.
}

void
OptimizationMultiAppCommandLineControl::execute()
{
  std::ostringstream oss;
  for (unsigned int i = 0; i < _parameters.size(); ++i)
  {
    ReporterName rname("FormFunction", _value_names[i]);
    const std::vector<Real> & value =
        getReporterValueByName<std::vector<Real>>(rname, REPORTER_MODE_REPLICATED);

    if (i > 0)
      oss << ";";
    oss << _parameters[i] << "="
        << "'";
    for (unsigned int v = 0; v < value.size(); ++v)
      oss << (v > 0 ? " " : "") << Moose::stringifyExact(value[v]);
    oss << "'";
  }

  std::vector<std::string> cli_args = {oss.str()};
  setControllableValueByName<std::vector<std::string>>(
      "MultiApp", _multi_app->name(), "cli_args", cli_args);
}
