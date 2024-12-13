//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SamplerTransientMultiApp.h"
#include "Sampler.h"
#include "StochasticToolsTransfer.h"
#include "SamplerFullSolveMultiApp.h"

registerMooseObject("StochasticToolsApp", SamplerTransientMultiApp);

InputParameters
SamplerTransientMultiApp::validParams()
{
  InputParameters params = TransientMultiApp::validParams();
  params += SamplerInterface::validParams();
  params.addClassDescription("Creates a sub-application for each row of each Sampler matrix.");
  params.addRequiredParam<SamplerName>("sampler",
                                       "The Sampler object to utilize for creating MultiApps.");
  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<Point>>("move_positions");
  params.suppressParameter<std::vector<unsigned int>>("move_apps");
  params.set<bool>("use_positions") = false;

  // use "batch-restore=2" to be consistent with SamplerFullSolveMultiApp and use the
  // allow_out_of_range flag to allow the StochasticToolsTransfer object to inspect the MultiApp
  // object parameters without triggering an assert.
  MooseEnum modes("normal=0 batch-reset=1 batch-restore=2", "normal");
  params.addParam<MooseEnum>(
      "mode",
      modes,
      "The operation mode, 'normal' creates one sub-application for each row in the Sampler and "
      "'batch-reset' and 'batch-restore' creates N sub-applications, where N is the minimum of "
      "'num_rows' in the Sampler and floor(number of processes / min_procs_per_app). To run "
      "the rows in the Sampler, 'batch-reset' will destroy and re-create sub-apps as needed, "
      "whereas the 'batch-restore' will backup and restore sub-apps to the initial state prior "
      "to execution, without destruction.");

  return params;
}

SamplerTransientMultiApp::SamplerTransientMultiApp(const InputParameters & parameters)
  : TransientMultiApp(parameters),
    SamplerInterface(this),
    _sampler(getSampler("sampler")),
    _mode(getParam<MooseEnum>("mode").getEnum<StochasticTools::MultiAppMode>()),
    _local_batch_app_index(0),
    _number_of_sampler_rows(_sampler.getNumberOfRows())
{
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESET)
    paramError("mode",
               "The supplied mode, '",
               getParam<MooseEnum>("mode"),
               "', currently is not implemented for the SamplerTransientMultiApp, the available "
               "options are 'normal' or 'batch-restore'.");

  if (getParam<unsigned int>("min_procs_per_app") !=
          _sampler.getParam<unsigned int>("min_procs_per_row") ||
      getParam<unsigned int>("max_procs_per_app") !=
          _sampler.getParam<unsigned int>("max_procs_per_row"))
    paramError("sampler",
               "Sampler and multiapp communicator configuration inconsistent. Please ensure that "
               "'MultiApps/",
               name(),
               "/min(max)_procs_per_app' and 'Samplers/",
               _sampler.name(),
               "/min(max)_procs_per_row' are the same.");

  init(_sampler.getNumberOfRows(),
       _sampler.getRankConfig(_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
                              _mode == StochasticTools::MultiAppMode::BATCH_RESTORE));
}

void
SamplerTransientMultiApp::initialSetup()
{
  TIME_SECTION("initialSetup", 2, "Setting Up SamplerTransientMultiApp");

  TransientMultiApp::initialSetup();

  // Perform initial backup for the batch sub-applications
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
  {
    dof_id_type n = _rank_config.num_local_sims;
    _batch_backup.resize(n);
    for (MooseIndex(n) i = 0; i < n; ++i)
      for (MooseIndex(_my_num_apps) j = 0; j < _my_num_apps; j++)
        _batch_backup[i].emplace_back(_apps[j]->backup());
  }
}

bool
SamplerTransientMultiApp::solveStep(Real dt, Real target_time, bool auto_advance)
{
  TIME_SECTION("solveStep", 3, "Solving SamplerTransientMultiApp");

  if (_sampler.getNumberOfRows() != _number_of_sampler_rows)
    mooseError("The size of the sampler has changed; SamplerTransientMultiApp object do not "
               "support dynamic Sampler output.");

  bool last_solve_converged = true;
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    last_solve_converged = solveStepBatch(dt, target_time, auto_advance);
  else
    last_solve_converged = TransientMultiApp::solveStep(dt, target_time, auto_advance);
  return last_solve_converged;
}

bool
SamplerTransientMultiApp::solveStepBatch(Real dt, Real target_time, bool auto_advance)
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
  {
    transfer->setGlobalMultiAppIndex(_rank_config.first_local_app_index);
    transfer->initializeToMultiapp();
  }
  for (auto transfer : from_transfers)
  {
    transfer->setGlobalMultiAppIndex(_rank_config.first_local_app_index);
    transfer->initializeFromMultiapp();
  }

  // Perform batch MultiApp solves
  _local_batch_app_index = 0;
  for (dof_id_type i = _rank_config.first_local_sim_index;
       i < _rank_config.first_local_sim_index + _rank_config.num_local_sims;
       ++i)
  {
    updateRowData(_local_batch_app_index);

    if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
      for (MooseIndex(_my_num_apps) j = 0; j < _my_num_apps; j++)
      {
        _apps[j]->restore(std::move(_batch_backup[_local_batch_app_index][j]), false);
        _apps[j]->finalizeRestore();
      }

    SamplerFullSolveMultiApp::execBatchTransfers(to_transfers,
                                                 i,
                                                 _row_data,
                                                 MultiAppTransfer::TO_MULTIAPP,
                                                 _fe_problem.verboseMultiApps(),
                                                 _console);

    // Set the file base based on the current row
    for (unsigned int ai = 0; ai < _my_num_apps; ++ai)
    {
      const std::string mname = getMultiAppName(name(), i, _number_of_sampler_rows);
      _apps[ai]->setOutputFileBase(_app.getOutputFileBase() + "_" + mname);
    }

    const bool curr_last_solve_converged =
        TransientMultiApp::solveStep(dt, target_time, auto_advance);
    last_solve_converged = last_solve_converged && curr_last_solve_converged;

    SamplerFullSolveMultiApp::execBatchTransfers(from_transfers,
                                                 i,
                                                 _row_data,
                                                 MultiAppTransfer::FROM_MULTIAPP,
                                                 _fe_problem.verboseMultiApps(),
                                                 _console);

    incrementTStep(target_time);

    if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
      for (MooseIndex(_my_num_apps) j = 0; j < _my_num_apps; j++)
        _batch_backup[_local_batch_app_index][j] = _apps[j]->backup();

    _local_batch_app_index++;
  }
  _local_batch_app_index = 0;

  // Finalize to/from transfers
  for (auto transfer : to_transfers)
    transfer->finalizeToMultiapp();
  for (auto transfer : from_transfers)
    transfer->finalizeFromMultiapp();

  return last_solve_converged;
}

std::vector<std::shared_ptr<StochasticToolsTransfer>>
SamplerTransientMultiApp::getActiveStochasticToolsTransfers(Transfer::DIRECTION direction)
{
  std::vector<std::shared_ptr<StochasticToolsTransfer>> output;
  const ExecuteMooseObjectWarehouse<Transfer> & warehouse =
      _fe_problem.getMultiAppTransferWarehouse(direction);
  for (std::shared_ptr<Transfer> transfer : warehouse.getActiveObjects())
  {
    std::shared_ptr<StochasticToolsTransfer> ptr =
        std::dynamic_pointer_cast<StochasticToolsTransfer>(transfer);
    if (ptr && ptr->getMultiApp().get() == this)
      output.push_back(ptr);
  }
  return output;
}

std::vector<std::string>
SamplerTransientMultiApp::getCommandLineArgs(const unsigned int local_app)
{
  std::vector<std::string> args;

  // With multiple processors per app, there are no local rows for non-root processors
  if (isRootProcessor())
  {
    // Since we only store param_names in cli_args, we need to find the values for each param from
    // sampler data and combine them to get full command line option strings.
    updateRowData(_mode == StochasticTools::MultiAppMode::NORMAL ? local_app
                                                                 : _local_batch_app_index);
    args = SamplerFullSolveMultiApp::sampledCommandLineArgs(
        _row_data, TransientMultiApp::getCommandLineArgs(local_app));
  }

  _my_communicator.broadcast(args);
  return args;
}

void
SamplerTransientMultiApp::updateRowData(dof_id_type local_index)
{
  if (!isRootProcessor())
    return;

  mooseAssert(local_index < _sampler.getNumberOfLocalRows(),
              "Local index must be less than number of local rows.");

  if (_row_data.empty() ||
      (_local_row_index == _sampler.getNumberOfLocalRows() - 1 && local_index == 0))
  {
    mooseAssert(local_index == 0,
                "The first time calling updateRowData must have a local index of 0.");
    _local_row_index = 0;
    _row_data = _sampler.getNextLocalRow();
  }
  else if (local_index - _local_row_index == 1)
  {
    _local_row_index++;
    _row_data = _sampler.getNextLocalRow();
  }

  mooseAssert(local_index == _local_row_index,
              "Local index must be equal or one greater than the index previously called.");
}
