//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
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
  params.addParam<bool>(
      "distribute_reporter_vector",
      false,
      "Transfer to/from a vector reporter from/to reporters on child applications. N "
      "to 1 or 1 to N type of transfer. The number of child applications must "
      "match the size of the vector reporter");
  return params;
}

MultiAppReporterTransfer::MultiAppReporterTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    ReporterTransferInterface(this),
    _from_reporter_names(getParam<std::vector<ReporterName>>("from_reporters")),
    _to_reporter_names(getParam<std::vector<ReporterName>>("to_reporters")),
    _subapp_index(getParam<unsigned int>("subapp_index")),
    _distribute_reporter_vector(getParam<bool>("distribute_reporter_vector"))
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
    else if (_directions.isValueSet(FROM_MULTIAPP) &&
             _subapp_index == std::numeric_limits<unsigned int>::max() &&
             multi_app->numGlobalApps() > 1 && !_distribute_reporter_vector)
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
  if (_directions.isValueSet(TO_MULTIAPP))
    problem_ptr = &getToMultiApp()->problemBase();
  else if (_subapp_index == std::numeric_limits<unsigned int>::max() &&
           getFromMultiApp()->hasLocalApp(0))
    problem_ptr = &getFromMultiApp()->appProblemBase(0);
  else if (getFromMultiApp()->hasLocalApp(_subapp_index))
    problem_ptr = &getFromMultiApp()->appProblemBase(_subapp_index);

  // Tell ReporterData to consume with replicated
  if (problem_ptr && !_distribute_reporter_vector)
    for (const auto & fn : _from_reporter_names)
      addReporterTransferMode(fn, REPORTER_MODE_REPLICATED, *problem_ptr);

  // Check that we have the correct reporter modes setup.
  if (_distribute_reporter_vector)
  {
    if (hasFromMultiApp())
      setVectorReporterTransferModes(getFromMultiApp(), _to_reporter_names, _from_reporter_names);
    else if (hasToMultiApp())
      setVectorReporterTransferModes(getToMultiApp(), _from_reporter_names, _to_reporter_names);
  }
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
      {
        if (_distribute_reporter_vector)
          transferFromVectorReporter(_from_reporter_names[n],
                                     _to_reporter_names[n],
                                     getToMultiApp()->problemBase(),
                                     getToMultiApp()->appProblemBase(ind),
                                     ind);
        else
          transferReporter(_from_reporter_names[n],
                           _to_reporter_names[n],
                           hasFromMultiApp() ? getFromMultiApp()->appProblemBase(ind)
                                             : getToMultiApp()->problemBase(),
                           getToMultiApp()->appProblemBase(ind));
      }
}

void
MultiAppReporterTransfer::executeFromMultiapp()
{
  if (!hasFromMultiApp())
    return;

  // subapp indices to perform transfers on
  std::vector<unsigned int> indices;
  if (_distribute_reporter_vector)
  {
    // If distributing, resize the indices vector to the number of global apps
    indices.resize(getFromMultiApp()->numGlobalApps());
    std::iota(indices.begin(), indices.end(), 0);
  }
  else if (_subapp_index == std::numeric_limits<unsigned int>::max())
  {
    // if _subapp_index not set indices is set to 0
    indices = {0};
  }
  else
    // set indices to specific _subapp_index
    indices = {_subapp_index};

  if (_distribute_reporter_vector)
    for (const auto n : index_range(_to_reporter_names))
    {
      // Clear all vector reporters and resize to the number of subapps.
      // The summing process later will make sure the reporter values are
      // consistent across the processors.
      auto size = getFromMultiApp()->numGlobalApps();
      clearVectorReporter(_to_reporter_names[n], getFromMultiApp()->problemBase());
      resizeReporter(_to_reporter_names[n], getFromMultiApp()->problemBase(), size);
    }

  for (const auto ind : indices)
    if (getFromMultiApp()->hasLocalApp(ind) &&
        (!hasToMultiApp() || getToMultiApp()->hasLocalApp(ind)))
      for (const auto n : index_range(_from_reporter_names))
      {
        if (_distribute_reporter_vector)
        {
          if (getFromMultiApp()->appProblemBase(ind).processor_id() == 0) // Subapp Root Rank only
            transferToVectorReporter(_from_reporter_names[n],
                                     _to_reporter_names[n],
                                     getFromMultiApp()->appProblemBase(ind),
                                     getFromMultiApp()->problemBase(),
                                     ind);
        }
        else
          transferReporter(_from_reporter_names[n],
                           _to_reporter_names[n],
                           getFromMultiApp()->appProblemBase(ind),
                           hasToMultiApp() ? getToMultiApp()->appProblemBase(ind) // !
                                           : getFromMultiApp()->problemBase());
      }

  if (_distribute_reporter_vector)
    for (const auto n : index_range(_to_reporter_names))
    {
      // Perform summing operation that makes sure all procs have the correct
      // Reporter values.
      sumVectorReporter(_to_reporter_names[n], getFromMultiApp()->problemBase());
    }
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

void
MultiAppReporterTransfer::checkSiblingsTransferSupported() const
{

  if (_distribute_reporter_vector)
    paramError("distribute_reporter_vector",
               "Distributing reporter vectors is not implemented with sibling transfers.");

  // Check that we are in the supported configuration: same number of source and target apps
  // The allocation of the child apps on the processors must be the same
  if (getFromMultiApp()->numGlobalApps() == getToMultiApp()->numGlobalApps())
  {
    for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
      if (getFromMultiApp()->hasLocalApp(i) + getToMultiApp()->hasLocalApp(i) == 1)
        mooseError("Child application allocation on parallel processes must be the same to support "
                   "siblings reporter transfer");
  }
  else
    mooseError("Number of source and target child apps must match for siblings transfer");
}

// Helper function to check reporter modes
void
MultiAppReporterTransfer::setVectorReporterTransferModes(
    const std::shared_ptr<MultiApp> & main_app,
    const std::vector<ReporterName> & main_app_rep_names,
    const std::vector<ReporterName> & sub_app_rep_names)
{
  // Set reporter transfer modes for the main app.
  for (const auto & rn : main_app_rep_names)
    addReporterTransferMode(rn, REPORTER_MODE_REPLICATED, main_app->problemBase());

  std::vector<unsigned int> indices(main_app->numGlobalApps());
  std::iota(indices.begin(), indices.end(), 0);

  // Set reporter transfer modes for sub app.
  // Setting to ROOT means this works for ROOT and REPLICATED reports with no
  // change to users.
  for (const auto & ind : indices)
    if (main_app->hasLocalApp(ind))
      for (const auto & rn : sub_app_rep_names)
        addReporterTransferMode(rn, REPORTER_MODE_ROOT, main_app->appProblemBase(ind));
}
