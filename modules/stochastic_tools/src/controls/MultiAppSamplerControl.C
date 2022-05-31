//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppSamplerControl.h"
#include "Function.h"
#include "Sampler.h"
#include "MultiApp.h"
#include "SamplerFullSolveMultiApp.h"

registerMooseObject("StochasticToolsApp", MultiAppSamplerControl);
registerMooseObjectRenamed("StochasticToolsApp",
                           MultiAppCommandLineControl,
                           "01/01/2023 00:00",
                           MultiAppSamplerControl);

InputParameters
MultiAppSamplerControl::validParams()
{
  InputParameters params = Control::validParams();
  params += SamplerInterface::validParams();
  params.addClassDescription("Control for modifying the command line arguments of MultiApps.");

  // Set and suppress the 'execute_on' flag, it doesn't work with any other flag
  params.set<ExecFlagEnum>("execute_on") = {EXEC_PRE_MULTIAPP_SETUP};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  params.addRequiredParam<MultiAppName>("multi_app", "The name of the MultiApp to control.");
  params.addRequiredParam<SamplerName>(
      "sampler",
      "The Sampler object to utilize for altering the command line options of the MultiApp.");
  params.addRequiredParam<std::vector<std::string>>(
      "param_names", "The names of the command line parameters to set via the sampled data.");

  return params;
}

MultiAppSamplerControl::MultiAppSamplerControl(const InputParameters & parameters)
  : Control(parameters),
    SamplerInterface(this),
    _multi_app(_fe_problem.getMultiApp(getParam<MultiAppName>("multi_app"))),
    _sampler(SamplerInterface::getSampler("sampler")),
    _param_names(getParam<std::vector<std::string>>("param_names"))
{
  if (!_sampler.getParam<ExecFlagEnum>("execute_on").contains(EXEC_PRE_MULTIAPP_SETUP))
    _sampler.paramError(
        "execute_on",
        "The sampler object, '",
        _sampler.name(),
        "', is being used by the '",
        name(),
        "' object, thus the 'execute_on' of the sampler must include 'PRE_MULTIAPP_SETUP'.");

  if (_multi_app->usingPositions())
    paramError("multi_app",
               "The MultiApp must construct its sub-apps in initial setup but not during its "
               "creation for '",
               type(),
               "'.\nTypically only sampler MultiApps work with '",
               type(),
               "' objects.");

  bool batch_reset = _multi_app->isParamValid("mode") &&
                     (_multi_app->getParam<MooseEnum>("mode") == "batch-reset");
  bool batch_restore = _multi_app->isParamValid("mode") &&
                       (_multi_app->getParam<MooseEnum>("mode") == "batch-restore");
  if (batch_reset)
    ; // Do not perform the App count test, because in batch mode there is only one

  else if (batch_restore)
    _multi_app->paramError(
        "mode",
        "The MultiApp object, '",
        _multi_app->name(),
        "', supplied to the '",
        name(),
        "' object is setup to run in 'batch-restore' mode, when using this mode command line "
        "arguments cannot be modified; batch-reset mode should be used instead.");

  else if (_multi_app->numGlobalApps() != _sampler.getNumberOfRows())
    mooseError("The number of sub apps (",
               _multi_app->numGlobalApps(),
               ") created by MultiApp object '",
               _multi_app->name(),
               "' must be equal to the number for rows (",
               _sampler.getNumberOfRows(),
               ") for the '",
               _sampler.name(),
               "' Sampler object.");
}

void
MultiAppSamplerControl::initialSetup()
{
  // Do not put anything here, this method is being called after execute because the execute_on
  // is set to PRE_MULTIAPP_SETUP for this class. It won't work any other way.
}

void
MultiAppSamplerControl::execute()
{
  // Gather the original arguments given in the parameter so we can keep them
  if (_orig_args.empty())
  {
    _orig_args = getControllableValueByName<std::vector<std::string>>(
        "MultiApp", _multi_app->name(), "cli_args", true);
    if (_orig_args.size() == 0)
      _orig_args.push_back("");
    else if (_param_names.size())
      for (auto & clia : _orig_args)
        clia += ";";
  }

  std::vector<std::string> cli_args = _orig_args;

  // To avoid storing duplicated param_names for each sampler, we store only param_names once in
  // "cli_args".

  // Handle a couple errors up front regarding bracket expressions
  bool has_brackets = false;
  if (_param_names.size())
  {
    has_brackets = _param_names[0].find("[") != std::string::npos;
    for (unsigned int i = 1; i < _param_names.size(); ++i)
      if (has_brackets != (_param_names[i].find("[") != std::string::npos))
        paramError("param_names",
                   "If the bracket is used, it must be provided to every parameter.");
  }
  if (!has_brackets && _sampler.getNumberOfCols() != _param_names.size())
    paramError("param_names",
               "The number of columns (",
               _sampler.getNumberOfCols(),
               ") must match the number of parameters (",
               _param_names.size(),
               ").");

  // Add the parameters that will be modified. The MultiApp will ultimately be
  // responsible for assigning the values from the sampler.
  std::ostringstream oss;
  for (dof_id_type col = 0; col < _param_names.size(); ++col)
  {
    if (col > 0)
      oss << ";";
    oss << _param_names[col];
  }

  // Put all the parameters in a single string
  for (auto & clia : cli_args)
    clia += oss.str();

  setControllableValueByName<std::vector<std::string>>(
      "MultiApp", _multi_app->name(), "cli_args", cli_args);
}
