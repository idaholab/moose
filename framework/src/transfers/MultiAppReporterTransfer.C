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
  InputParameters params = MultiAppReporterTransferBase::validParams();
  params.addClassDescription(
      "Transfers reporter data between the main application and sub-application(s).");
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
  : MultiAppReporterTransferBase(parameters),
    _from_reporter_names(getParam<std::vector<ReporterName>>("from_reporters")),
    _to_reporter_names(getParam<std::vector<ReporterName>>("to_reporters")),
    _subapp_index(getParam<unsigned int>("subapp_index"))
{
  if (_from_reporter_names.size() != _to_reporter_names.size())
    paramError("to_reporters", "from_reporters and to_reporters must be the same size.");

  if (_directions.size() > 1)
    paramError("direction", "This transfer only supports a single direction.");

  // Errors for sub app index.
  if (_subapp_index != std::numeric_limits<unsigned int>::max() &&
      _subapp_index >= _multi_app->numGlobalApps())
    paramError(
        "subapp_index",
        "The supplied sub-application index is greater than the number of sub-applications.");
  else if (_directions.contains(FROM_MULTIAPP) &&
           _subapp_index == std::numeric_limits<unsigned int>::max() &&
           _multi_app->numGlobalApps() > 1)
    paramError("multi_app", "subapp_index must be provided when more than one subapp is present.");
}

void
MultiAppReporterTransfer::initialSetup()
{
  // We need to get a reference to the data now so we can tell ReporterData
  // that we consume a replicated version.
  // Find proper index
  int ind;
  if (_directions.contains(TO_MULTIAPP))
    ind = std::numeric_limits<unsigned int>::max();
  else if (_subapp_index == std::numeric_limits<unsigned int>::max())
    ind = 0;
  else
    ind = _subapp_index;
  // Tell ReporterData to consume with replicated
  for (const auto & fn : _from_reporter_names)
    addReporterTransferMode(fn, REPORTER_MODE_REPLICATED, ind);
}

void
MultiAppReporterTransfer::executeToMultiapp()
{
  for (unsigned int n = 0; n < _from_reporter_names.size(); ++n)
  {
    if (_subapp_index == std::numeric_limits<unsigned int>::max())
    {
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
        this->transferToMultiApp(_from_reporter_names[n], _to_reporter_names[n], i);
    }

    else
      this->transferToMultiApp(_from_reporter_names[n], _to_reporter_names[n], _subapp_index);
  }
}

void
MultiAppReporterTransfer::executeFromMultiapp()
{
  unsigned int ind = _subapp_index != std::numeric_limits<unsigned int>::max() ? _subapp_index : 0;
  for (unsigned int n = 0; n < _from_reporter_names.size(); ++n)
    this->transferFromMultiApp(_from_reporter_names[n], _to_reporter_names[n], ind);
}
