//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SamplerFullSolveMultiApp.h"
#include "Sampler.h"
#include "StochasticToolsTransfer.h"

registerMooseObject("StochasticToolsApp", SamplerFullSolveMultiApp);

template <>
InputParameters
validParams<SamplerFullSolveMultiApp>()
{
  InputParameters params = validParams<FullSolveMultiApp>();
  params += validParams<SamplerInterface>();
  params.addClassDescription(
      "Creates a full-solve type sub-application for each row of each Sampler matrix.");
  params.addParam<SamplerName>("sampler", "The Sampler object to utilize for creating MultiApps.");
  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<Point>>("move_positions");
  params.suppressParameter<std::vector<unsigned int>>("move_apps");
  params.set<bool>("use_positions") = false;

  MooseEnum modes("normal=0 batch-reset=1 batch-restore=2", "normal");
  params.addParam<MooseEnum>(
      "mode",
      modes,
      "The operation mode, 'normal' creates one sub-application for each row in the Sampler and "
      "'batch' creates on sub-application for each processor and re-executes for each row.");

  return params;
}

SamplerFullSolveMultiApp::SamplerFullSolveMultiApp(const InputParameters & parameters)
  : FullSolveMultiApp(parameters),
    SamplerInterface(this),
    _sampler(SamplerInterface::getSampler("sampler")),
    _mode(getParam<MooseEnum>("mode").getEnum<StochasticTools::MultiAppMode>()),
    _local_batch_app_index(0)
{
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
      _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    init(n_processors());
  else
    init(_sampler.getTotalNumberOfRows());
}

bool
SamplerFullSolveMultiApp::solveStep(Real dt, Real target_time, bool auto_advance)
{
  bool last_solve_converged = true;
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
      _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    last_solve_converged = solveStepBatch(dt, target_time, auto_advance);
  else
    last_solve_converged = FullSolveMultiApp::solveStep(dt, target_time, auto_advance);
  return last_solve_converged;
}

bool
SamplerFullSolveMultiApp::solveStepBatch(Real dt, Real target_time, bool auto_advance)
{
  // Value to return
  bool last_solve_converged = true;

  // List of active relevant Transfer objects
  std::vector<std::shared_ptr<StochasticToolsTransfer>> to_transfers =
      getActiveStochasticToolsTransfers(MultiAppTransfer::TO_MULTIAPP);
  std::vector<std::shared_ptr<StochasticToolsTransfer>> from_transfers =
      getActiveStochasticToolsTransfers(MultiAppTransfer::FROM_MULTIAPP);

  // Initialize to/from transfers
  for (auto transfer : to_transfers)
    transfer->initializeToMultiapp();
  for (auto transfer : from_transfers)
    transfer->initializeFromMultiapp();

  if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    backup();

  // Perform batch MultiApp solves
  _local_batch_app_index = 0;
  dof_id_type num_items = _sampler.getLocalNumerOfRows();
  for (MooseIndex(num_items) i = 0; i < num_items; ++i)
  {
    for (auto & transfer : to_transfers)
      transfer->executeToMultiapp();

    last_solve_converged = FullSolveMultiApp::solveStep(dt, target_time, auto_advance);

    for (auto & transfer : from_transfers)
      transfer->executeFromMultiapp();

    if (i != num_items - 1)
    {
      if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
        restore();
      else
      {
        // The app is being reset for the next loop, thus the batch index must be indexed as such
        _local_batch_app_index = i + 1;
        for (std::size_t app = 0; app < _total_num_apps; app++)
          resetApp(app, target_time);
        initialSetup();
      }
    }
  }

  // Finalize to/from transfers
  for (auto transfer : to_transfers)
    transfer->finalizeToMultiapp();
  for (auto transfer : from_transfers)
    transfer->finalizeFromMultiapp();

  return last_solve_converged;
}

std::vector<std::shared_ptr<StochasticToolsTransfer>>
SamplerFullSolveMultiApp::getActiveStochasticToolsTransfers(MultiAppTransfer::DIRECTION direction)
{
  std::vector<std::shared_ptr<StochasticToolsTransfer>> output;
  const ExecuteMooseObjectWarehouse<Transfer> & warehouse =
      _fe_problem.getMultiAppTransferWarehouse(direction);
  for (std::shared_ptr<Transfer> transfer : warehouse.getActiveObjects())
  {
    auto ptr = std::dynamic_pointer_cast<StochasticToolsTransfer>(transfer);
    if (ptr)
      output.push_back(ptr);
  }
  return output;
}

std::string
SamplerFullSolveMultiApp::getCommandLineArgsParamHelper(unsigned int local_app)
{
  // Since we only store param_names in cli_args, we need to find the values for each param from
  // sampler data and combine them to get full command line option strings.

  std::vector<DenseMatrix<Real>> & samples = _sampler.getSamples();
  std::ostringstream oss;
  Sampler::Location loc =
      _sampler.getLocation(_local_batch_app_index + _sampler.getLocalRowBegin());
  DenseMatrix<Real> & matrix = samples[loc.sample()];

  const std::vector<std::string> & cli_args_name = MooseUtils::split(_cli_args[loc.sample()], ";");

  unsigned int local_i = 0;
  if (_sampler.isSampleDataDistributed())
  {
    for (unsigned int i = 0; i < loc.sample(); ++i)
      local_i += samples[i].m();

    local_i = _local_batch_app_index - local_i;
  }

  for (unsigned int col = 0; col < matrix.n(); ++col)
  {
    if (col > 0)
      oss << ";";

    if (_mode == StochasticTools::MultiAppMode::BATCH_RESET)
    {
      if (_sampler.isSampleDataDistributed())
        oss << cli_args_name[col] << "=" << Moose::stringify(matrix(local_i, col));
      else
      {
        oss << cli_args_name[col] << "=" << Moose::stringify(matrix(loc.row(), col));
        if (loc.row() >= matrix.m())
          oss.str("");
      }
    }
    else
    {
      oss << cli_args_name[col] << "="
          << Moose::stringify(matrix(local_app + _first_local_app, col));

      if ((local_app + _first_local_app) >= matrix.m())
        oss.str("");
    }
  }

  return oss.str();
}
