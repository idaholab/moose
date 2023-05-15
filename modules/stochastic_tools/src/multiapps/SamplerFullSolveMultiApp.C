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
#include "Console.h"
#include "VariadicTable.h"

registerMooseObject("StochasticToolsApp", SamplerFullSolveMultiApp);

InputParameters
SamplerFullSolveMultiApp::validParams()
{
  InputParameters params = FullSolveMultiApp::validParams();
  params += SamplerInterface::validParams();
  params += ReporterInterface::validParams();
  params.addClassDescription(
      "Creates a full-solve type sub-application for each row of each Sampler matrix.");
  params.addRequiredParam<SamplerName>("sampler",
                                       "The Sampler object to utilize for creating MultiApps.");
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
      "'batch-reset' and 'batch-restore' creates N sub-applications, where N is the minimum of "
      "'num_rows' in the Sampler and floor(number of processes / min_procs_per_app). To run "
      "the rows in the Sampler, 'batch-reset' will destroy and re-create sub-apps as needed, "
      "whereas the 'batch-restore' will backup and restore sub-apps to the initial state prior "
      "to execution, without destruction.");
  params.addParam<ReporterName>(
      "should_run_reporter",
      "Vector reporter value determining whether a certain multiapp should be run with this "
      "multiapp. This only works in batch-reset or batch-restore mode.");
  return params;
}

SamplerFullSolveMultiApp::SamplerFullSolveMultiApp(const InputParameters & parameters)
  : FullSolveMultiApp(parameters),
    SamplerInterface(this),
    ReporterInterface(this),
    _sampler(getSampler("sampler")),
    _mode(getParam<MooseEnum>("mode").getEnum<StochasticTools::MultiAppMode>()),
    _local_batch_app_index(0),
    _solved_once(false)
{
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
  _number_of_sampler_rows = _sampler.getNumberOfRows();

  if (isParamValid("should_run_reporter") && _mode == StochasticTools::MultiAppMode::NORMAL)
    paramError("should_run_reporter",
               "Conditionally run sampler multiapp only works in batch modes.");
}

void SamplerFullSolveMultiApp::preTransfer(Real /*dt*/, Real /*target_time*/)
{
  // Reinitialize MultiApp size
  const auto num_rows = _sampler.getNumberOfRows();
  if (num_rows != _number_of_sampler_rows)
  {
    init(num_rows,
         _sampler.getRankConfig(_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
                                _mode == StochasticTools::MultiAppMode::BATCH_RESTORE));
    _number_of_sampler_rows = num_rows;
    _row_data.clear();
  }

  // Reinitialize app to original state prior to solve, if a solve has occured.
  // Since the app is reinitialized in the solve step either way, we skip this
  // for batch-reset mode.
  if (_solved_once && _mode != StochasticTools::MultiAppMode::BATCH_RESET)
    initialSetup();

  if (isParamValid("should_run_reporter"))
    _should_run = &getReporterValue<std::vector<bool>>("should_run_reporter");
}

bool
SamplerFullSolveMultiApp::solveStep(Real dt, Real target_time, bool auto_advance)
{
  TIME_SECTION("solveStep", 3, "Solving SamplerFullSolveMultiApp");

  mooseAssert(_my_num_apps, _sampler.getNumberOfLocalRows());

  bool last_solve_converged = true;

  if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
      _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    last_solve_converged = solveStepBatch(dt, target_time, auto_advance);
  else
    last_solve_converged = FullSolveMultiApp::solveStep(dt, target_time, auto_advance);

  _solved_once = true;

  return last_solve_converged;
}

bool
SamplerFullSolveMultiApp::solveStepBatch(Real dt, Real target_time, bool auto_advance)
{
  TIME_SECTION("solveStepBatch", 3, "Solving Step Batch For SamplerFullSolveMultiApp");

  if (_should_run && _should_run->size() < _sampler.getNumberOfLocalRows())
    paramError("should_run_reporter",
               "Reporter deteriming multiapp run must be of size greater than or equal to the "
               "number of local rows in the sampler, ",
               _should_run->size(),
               " < ",
               _sampler.getNumberOfLocalRows(),
               ".");

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

  if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    backup();

  // Perform batch MultiApp solves
  _local_batch_app_index = 0;
  for (dof_id_type i = _rank_config.first_local_sim_index;
       i < _rank_config.first_local_sim_index + _rank_config.num_local_sims;
       ++i)
  {
    updateRowData(_local_batch_app_index);

    bool run = true;
    if (_should_run)
    {
      if (isRootProcessor())
        run = (*_should_run)[_local_batch_app_index];
      _my_communicator.broadcast(run, 0);
    }
    if (!run)
    {
      _local_batch_app_index++;
      continue;
    }

    // Given that we don't initialize in preTransfer for batch-reset mode, we need
    // a different logic for resetting the apps for every sample:
    // - batch-restore: after (re-)initializing the problem, we only need to restore
    //   starting from the second sample
    if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    {
      if (i != _rank_config.first_local_sim_index)
        restore();
    }
    // - batch-reset: we don't need to initialize for the first sample in the first
    //   solve. After that, we initialize every time. This is mainly to avoid unnecessary
    //   initializations for cases when the multiapp does not need to be executed (conditional runs)
    else
    {
      if (i != _rank_config.first_local_sim_index || _solved_once)
        initialSetup();
    }

    execBatchTransfers(to_transfers,
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
        FullSolveMultiApp::solveStep(dt, target_time, auto_advance);
    last_solve_converged = last_solve_converged && curr_last_solve_converged;

    execBatchTransfers(from_transfers,
                       i,
                       _row_data,
                       MultiAppTransfer::FROM_MULTIAPP,
                       _fe_problem.verboseMultiApps(),
                       _console);

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

void
SamplerFullSolveMultiApp::execBatchTransfers(
    const std::vector<std::shared_ptr<StochasticToolsTransfer>> & transfers,
    dof_id_type global_row_index,
    const std::vector<Real> & row_data,
    Transfer::DIRECTION direction,
    bool verbose,
    const ConsoleStream & console)
{
  if (verbose && transfers.size())
  {
    console << COLOR_CYAN << "\nBatch transfers for row " << global_row_index;
    if (direction == MultiAppTransfer::TO_MULTIAPP)
      console << " To ";
    else if (direction == MultiAppTransfer::FROM_MULTIAPP)
      console << " From ";
    console << "MultiApps" << COLOR_DEFAULT << ":" << std::endl;

    console << "Sampler row " << global_row_index << " data: [" << Moose::stringify(row_data) << "]"
            << std::endl;

    // Build Table of Transfer Info
    VariadicTable<std::string, std::string, std::string, std::string> table(
        {"Name", "Type", "From", "To"});
    for (const auto & transfer : transfers)
      table.addRow(
          transfer->name(), transfer->type(), transfer->getFromName(), transfer->getToName());
    table.print(console);
  }

  for (auto & transfer : transfers)
  {
    transfer->setGlobalRowIndex(global_row_index);
    transfer->setCurrentRow(row_data);
    if (direction == MultiAppTransfer::TO_MULTIAPP)
      transfer->executeToMultiapp();
    else if (direction == MultiAppTransfer::FROM_MULTIAPP)
      transfer->executeFromMultiapp();
  }

  if (verbose && transfers.size())
    console << COLOR_CYAN << "Batch transfers for row " << global_row_index << " Are Finished\n"
            << COLOR_DEFAULT << std::endl;
}

void
SamplerFullSolveMultiApp::showStatusMessage(unsigned int i) const
{
  // Local row is the app index if in normal mode, otherwise it's _local_batch_app_index
  const dof_id_type local_row =
      _mode == StochasticTools::MultiAppMode::NORMAL ? (dof_id_type)i : _local_batch_app_index;
  // If the local row is less than the number of local sims, we aren't finished yet
  if (local_row < _rank_config.num_local_sims - 1)
    return;

  // Loop through processors to communicate completeness
  for (const auto & pid : make_range(n_processors()))
  {
    // This is what is being sent to trigger completeness
    dof_id_type last_row = _rank_config.is_first_local_rank
                               ? _rank_config.first_local_sim_index + _rank_config.num_local_sims
                               : 0;
    // Cannot send/receive to the same processor, so avoid if root
    if (pid > 0)
    {
      // Send data to root
      if (pid == processor_id())
        _communicator.send(0, last_row);
      // Receive data from source
      else if (processor_id() == 0)
        _communicator.receive(pid, last_row);
    }

    // Output the samples that are complete if it's the main processor for the batch
    if (last_row)
      _console << COLOR_CYAN << type() << " [" << name() << "] " << last_row << "/"
               << _number_of_sampler_rows << " samples complete!" << std::endl;
  }
}

std::vector<std::shared_ptr<StochasticToolsTransfer>>
SamplerFullSolveMultiApp::getActiveStochasticToolsTransfers(Transfer::DIRECTION direction)
{
  std::vector<std::shared_ptr<StochasticToolsTransfer>> output;
  const ExecuteMooseObjectWarehouse<Transfer> & warehouse =
      _fe_problem.getMultiAppTransferWarehouse(direction);
  for (std::shared_ptr<Transfer> transfer : warehouse.getActiveObjects())
  {
    auto ptr = std::dynamic_pointer_cast<StochasticToolsTransfer>(transfer);
    if (ptr && ptr->getMultiApp().get() == this)
      output.push_back(ptr);
  }
  return output;
}

std::string
SamplerFullSolveMultiApp::getCommandLineArgsParamHelper(unsigned int local_app)
{
  std::string args;

  // With multiple processors per app, there are no local rows for non-root processors
  if (isRootProcessor())
  {
    // Since we only store param_names in cli_args, we need to find the values for each param from
    // sampler data and combine them to get full command line option strings.
    updateRowData(_mode == StochasticTools::MultiAppMode::NORMAL ? local_app
                                                                 : _local_batch_app_index);

    const std::vector<std::string> & full_args_name =
        MooseUtils::split(FullSolveMultiApp::getCommandLineArgsParamHelper(local_app), ";");
    args = sampledCommandLineArgs(_row_data, full_args_name);
  }

  _my_communicator.broadcast(args);
  return args;
}

void
SamplerFullSolveMultiApp::updateRowData(dof_id_type local_index)
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

std::string
SamplerFullSolveMultiApp::sampledCommandLineArgs(const std::vector<Real> & row,
                                                 const std::vector<std::string> & full_args_name)
{
  std::ostringstream oss;

  // Find parameters that are meant to be assigned by sampler values
  std::vector<std::string> cli_args_name;
  for (const auto & fan : full_args_name)
  {
    // If it has an '=', then it is not meant to be modified
    if (fan.find("=") == std::string::npos)
      cli_args_name.push_back(fan);
    else
      oss << fan << ";";
  }

  // Make sure the parameters either all have brackets, or none of them do
  bool has_brackets = false;
  if (cli_args_name.size())
  {
    has_brackets = cli_args_name[0].find("[") != std::string::npos;
    for (unsigned int i = 1; i < cli_args_name.size(); ++i)
      if (has_brackets != (cli_args_name[i].find("[") != std::string::npos))
        ::mooseError("If the bracket is used, it must be provided to every parameter.");
  }
  if (!has_brackets && cli_args_name.size() && cli_args_name.size() != row.size())
    ::mooseError("Number of command line arguments does not match number of sampler columns.");

  for (unsigned int i = 0; i < cli_args_name.size(); ++i)
  {
    // Assign bracketed parameters
    if (has_brackets)
    {
      // Split param name and vector assignment: "param[0,(3.14),1]" -> {"param", "0,(3.14),1]"}
      const std::vector<std::string> & vector_param = MooseUtils::split(cli_args_name[i], "[");
      // Get inices of vector: "0,(3.14),1]" -> {"0", "(3.14)", "1"}
      const std::vector<std::string> & index_string =
          MooseUtils::split(vector_param[1].substr(0, vector_param[1].find("]")), ",");

      // Loop through indices and assign parameter: param='row[0] 3.14 row[1]'
      oss << vector_param[0] << "='";
      std::string sep = "";
      for (const auto & istr : index_string)
      {
        oss << sep;
        sep = " ";
        // If the value is enclosed in parentheses, then it isn't an index, it's a value
        if (istr.find("(") != std::string::npos)
          oss << std::stod(istr.substr(istr.find("(") + 1));
        // Assign the value from row if it is an index
        else
        {
          unsigned int index = MooseUtils::stringToInteger(istr);
          if (index >= row.size())
            ::mooseError("The provided global column index (",
                         index,
                         ") for ",
                         vector_param[0],
                         " is out of bound.");
          oss << Moose::stringifyExact(row[index]);
        }
      }
      oss << "';";
    }
    // Assign scalar parameters
    else
    {
      oss << cli_args_name[i] << "=" << Moose::stringifyExact(row[i]) << ";";
    }
  }

  return oss.str();
}
