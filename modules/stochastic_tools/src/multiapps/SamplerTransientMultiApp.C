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

registerMooseObject("StochasticToolsApp", SamplerTransientMultiApp);

InputParameters
SamplerTransientMultiApp::validParams()
{
  InputParameters params = TransientMultiApp::validParams();
  params += SamplerInterface::validParams();
  params.addClassDescription("Creates a sub-application for each row of each Sampler matrix.");
  params.addParam<SamplerName>("sampler", "The Sampler object to utilize for creating MultiApps.");
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
      "'batch' creates on sub-application for each processor and re-executes for each row.");

  return params;
}

SamplerTransientMultiApp::SamplerTransientMultiApp(const InputParameters & parameters)
  : TransientMultiApp(parameters),
    SamplerInterface(this),
    _sampler(SamplerInterface::getSampler("sampler")),
    _mode(getParam<MooseEnum>("mode").getEnum<StochasticTools::MultiAppMode>()),
    _number_of_sampler_rows(_sampler.getNumberOfRows()),
    _perf_solve_step(registerTimedSection("solveStep", 1)),
    _perf_solve_batch_step(registerTimedSection("solveStepBatch", 1)),
    _perf_initial_setup(registerTimedSection("initialSetup", 2)),
    _perf_command_line_args(registerTimedSection("getCommandLineArgsParamHelper", 4))
{
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
  {
    if (n_processors() > _sampler.getNumberOfRows())
      paramError(
          "mode",
          "There appears to be more available processors (",
          n_processors(),
          ") than samples (",
          _sampler.getNumberOfRows(),
          "), this is not supported in "
          "batch mode. Consider switching to \'normal\' to allow multiple processors per sample.");
    init(n_processors());
  }
  else if (_mode == StochasticTools::MultiAppMode::NORMAL)
    init(_sampler.getNumberOfRows());
  else
    paramError("mode",
               "The supplied mode, '",
               getParam<MooseEnum>("mode"),
               "', currently is not implemented for the SamplerTransientMultiApp, the available "
               "options are 'normal' or 'batch-restore'.");
}

void
SamplerTransientMultiApp::initialSetup()
{
  TIME_SECTION(_perf_initial_setup);

  TransientMultiApp::initialSetup();

  // Perform initial backup for the batch sub-applications
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
  {
    dof_id_type n = _sampler.getNumberOfLocalRows();
    _batch_backup.resize(n);
    for (MooseIndex(n) i = 0; i < n; ++i)
      for (MooseIndex(_my_num_apps) j = 0; j < _my_num_apps; j++)
        _batch_backup[i].emplace_back(_apps[j]->backup());
  }
}

bool
SamplerTransientMultiApp::solveStep(Real dt, Real target_time, bool auto_advance)
{
  TIME_SECTION(_perf_solve_step);

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
  TIME_SECTION(_perf_solve_batch_step);

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

  // Perform batch MultiApp solves
  dof_id_type num_items = _sampler.getNumberOfLocalRows();
  for (MooseIndex(num_items) i = 0; i < num_items; ++i)
  {
    if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
      for (MooseIndex(_my_num_apps) j = 0; j < _my_num_apps; j++)
        _apps[j]->restore(_batch_backup[i][j]);

    for (auto transfer : to_transfers)
      transfer->executeToMultiapp();

    last_solve_converged = TransientMultiApp::solveStep(dt, target_time, auto_advance);

    for (auto transfer : from_transfers)
      transfer->executeFromMultiapp();

    if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
      for (MooseIndex(_my_num_apps) j = 0; j < _my_num_apps; j++)
        _batch_backup[i][j] = _apps[j]->backup();
  }

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

std::string
SamplerTransientMultiApp::getCommandLineArgsParamHelper(unsigned int local_app)
{
  TIME_SECTION(_perf_command_line_args);

  std::string args;

  // With multiple processors per app, there are no local rows for non-root processors
  if (isRootProcessor())
  {
    // Since we only store param_names in cli_args, we need to find the values for each param from
    // sampler data and combine them to get full command line option strings.
    std::vector<Real> row = _sampler.getNextLocalRow();

    std::ostringstream oss;
    const std::vector<std::string> & cli_args_name =
        MooseUtils::split(TransientMultiApp::getCommandLineArgsParamHelper(local_app), ";");

    bool has_brackets = false;
    if (cli_args_name.size())
    {
      has_brackets = cli_args_name[0].find("[") != std::string::npos;
      for (unsigned int i = 1; i < cli_args_name.size(); ++i)
        if (has_brackets != (cli_args_name[i].find("[") != std::string::npos))
          mooseError("If the bracket is used, it must be provided to every parameter.");
    }
    if (!has_brackets && cli_args_name.size() != _sampler.getNumberOfCols())
      mooseError("Number of command line arguments does not match number of sampler columns.");

    for (unsigned int i = 0; i < cli_args_name.size(); ++i)
    {
      if (has_brackets)
      {
        const std::vector<std::string> & vector_param = MooseUtils::split(cli_args_name[i], "[");
        const std::vector<std::string> & index_string =
            MooseUtils::split(vector_param[1].substr(0, vector_param[1].find("]")), ",");

        oss << vector_param[0] << "='";
        std::vector<unsigned int> col_count;
        for (unsigned j = 0; j < index_string.size(); ++j)
        {
          if (index_string[j].find("(") != std::string::npos)
            oss << std::stod(index_string[j].substr(index_string[j].find("(") + 1));
          else
          {
            unsigned int index = MooseUtils::stringToInteger(index_string[j]);
            if (index >= row.size())
              mooseError("The provided global column index (",
                         index,
                         ") for ",
                         vector_param[0],
                         " is out of bound.");
            oss << Moose::stringify(row[index]);
            if (std::find(col_count.begin(), col_count.end(), index) == col_count.end())
              col_count.push_back(index);
          }
          if (j != index_string.size() - 1)
            oss << " ";
        }
        oss << "';";
      }
      else
      {
        oss << cli_args_name[i] << "=" << Moose::stringify(row[i]) << ";";
      }
    }

    args = oss.str();
  }

  _my_communicator.broadcast(args);

  return args;
}
