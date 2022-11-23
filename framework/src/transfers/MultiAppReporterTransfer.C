//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppReporterTransfer.h"
#include "MultiApp.h"

registerMooseObject("MooseApp", MultiAppReporterTransfer);

InputParameters
MultiAppReporterTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params += ReporterTransferInterface::validParams();
  params.addClassDescription("Transfers reporter data between two applications.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "from_reporters",
      "List of the reporter names (object_name/value_name) to transfer the value from.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "to_reporters",
      "List of the reporter names (object_name/value_name) to transfer the value to.");
  params.addParam<unsigned int>(
      "subapp_index",
      std::numeric_limits<unsigned int>::max(),
      "The MultiApp object sub-application index to use when transferring to/from the "
      "sub-application. If unset and transferring to the sub-applications then all "
      "sub-applications will receive data. The value must be set when transferring from a "
      "sub-application.");
  return params;
}

MultiAppReporterTransfer::MultiAppReporterTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    ReporterTransferInterface(this),
    _from_reporter_names(getParam<std::vector<ReporterName>>("from_reporters")),
    _to_reporter_names(getParam<std::vector<ReporterName>>("to_reporters")),
    _subapp_index(getParam<unsigned int>("subapp_index"))
{
  if (_from_reporter_names.size() != _to_reporter_names.size())
    paramError("to_reporters", "from_reporters and to_reporters must be the same size.");

  if (_directions.size() > 1)
    paramError("direction", "This transfer only supports a single direction.");

  if (isParamValid("to_multi_app") && isParamValid("from_multi_app") &&
      _subapp_index != std::numeric_limits<unsigned int>::max())
    paramError("subapp_index",
               "The subapp_index parameter is not supported for transfers between two multiapps");

  if (hasFromMultiApp())
  {
    const auto multi_app = getFromMultiApp();
    // Errors for sub app index.
    if (_subapp_index != std::numeric_limits<unsigned int>::max() &&
        _subapp_index >= multi_app->numGlobalApps())
      paramError(
          "subapp_index",
          "The supplied sub-application index is greater than the number of sub-applications.");
    else if (_directions.contains(FROM_MULTIAPP) &&
             _subapp_index == std::numeric_limits<unsigned int>::max() &&
             multi_app->numGlobalApps() > 1)
      paramError("from_multi_app",
                 "subapp_index must be provided when more than one subapp is present.");
  }
  else if (hasToMultiApp())
  {
    // Errors for sub app index.
    if (_subapp_index != std::numeric_limits<unsigned int>::max() &&
        _subapp_index >= getToMultiApp()->numGlobalApps())
      paramError(
          "subapp_index",
          "The supplied sub-application index is greater than the number of sub-applications.");
  }
}

void
MultiAppReporterTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();

  // We need to get a reference to the data now so we can tell ReporterData
  // that we consume a replicated version.
  // Find proper FEProblem
  FEProblemBase * problem_ptr = nullptr;
  if (_directions.contains(TO_MULTIAPP))
    problem_ptr = &getToMultiApp()->problemBase();
  else if (_subapp_index == std::numeric_limits<unsigned int>::max() &&
           getFromMultiApp()->hasLocalApp(0))
    problem_ptr = &getFromMultiApp()->appProblemBase(0);
  else if (getFromMultiApp()->hasLocalApp(_subapp_index))
    problem_ptr = &getFromMultiApp()->appProblemBase(_subapp_index);

  // Tell ReporterData to consume with replicated
  if (problem_ptr)
    for (const auto & fn : _from_reporter_names)
      addReporterTransferMode(fn, REPORTER_MODE_REPLICATED, *problem_ptr);
}

void
MultiAppReporterTransfer::executeToMultiapp()
{
  if (!hasToMultiApp())
    return;

  std::vector<unsigned int> indices;
  if (_subapp_index == std::numeric_limits<unsigned int>::max())
  {
    indices.resize(getToMultiApp()->numGlobalApps());
    std::iota(indices.begin(), indices.end(), 0);
  }
  else
    indices = {_subapp_index};

  for (const auto & ind : indices)
    if (getToMultiApp()->hasLocalApp(ind) &&
        (!hasFromMultiApp() || getFromMultiApp()->hasLocalApp(ind)))
      for (unsigned int n = 0; n < _from_reporter_names.size(); ++n)
        transferReporter(_from_reporter_names[n],
                         _to_reporter_names[n],
                         hasFromMultiApp() ? getFromMultiApp()->appProblemBase(ind)
                                           : getToMultiApp()->problemBase(),
                         getToMultiApp()->appProblemBase(ind));
}

void
MultiAppReporterTransfer::executeFromMultiapp()
{
  if (!hasFromMultiApp())
    return;

  unsigned int ind = _subapp_index != std::numeric_limits<unsigned int>::max() ? _subapp_index : 0;
  if (getFromMultiApp()->hasLocalApp(ind) &&
      (!hasToMultiApp() || getToMultiApp()->hasLocalApp(ind)))
    for (unsigned int n = 0; n < _from_reporter_names.size(); ++n)
      transferReporter(_from_reporter_names[n],
                       _to_reporter_names[n],
                       getFromMultiApp()->appProblemBase(ind),
                       hasToMultiApp() ? getToMultiApp()->appProblemBase(ind)
                                       : getFromMultiApp()->problemBase());
}

void
MultiAppReporterTransfer::execute()
{
  TIME_SECTION("MultiAppReporterTransfer::execute()", 5, "Transferring reporters");

  if (_current_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else
    executeToMultiapp();
}
