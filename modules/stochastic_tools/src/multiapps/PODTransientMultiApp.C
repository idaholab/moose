//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "PODTransientMultiApp.h"
#include "Sampler.h"
#include "StochasticToolsTransfer.h"

registerMooseObject("StochasticToolsApp", PODTransientMultiApp);

InputParameters
PODTransientMultiApp::validParams()
{
  InputParameters params = SamplerTransientMultiApp::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription(
      "Creates amultiapp which gathers snapshots and residuals for a POD surrogate.");
  params.addRequiredParam<UserObjectName>(
      "trainer_name", "Trainer object that contains the solutions for different samples.");
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(StochasticTools::EXEC_POST_SNAPSHOT_GEN);

  return params;
}

PODTransientMultiApp::PODTransientMultiApp(const InputParameters & parameters)
  : SamplerTransientMultiApp(parameters),
    SurrogateModelInterface(this),
    _trainer(getSurrogateTrainer<PODReducedBasisTrainer>("trainer_name"))
{
}

void
PODTransientMultiApp::preTransfer(Real dt, Real target_time)
{
  // Reinitialize the problem only if the snapshot generation part is done.
  // This check has to be switched to simething similar to the one in PODFullSolve Multiapp,
  // since the current flag is always NONE. Right now this is used to protect agains moving
  // past the snapsot generation phase.
  if (_fe_problem.getCurrentExecuteOnFlag() == StochasticTools::EXEC_POST_SNAPSHOT_GEN)
  {
    dof_id_type base_size = _trainer.getSumBaseSize();
    if (base_size < 1)
      mooseError(
          "There are no basis vectors available for residual generation."
          " This indicates that the bases have not been created yet."
          " The most common cause of this error is the wrong setting"
          " of the 'execute_on' flags in the PODFullSolveMultiApp and/or PODReducedBasisTrainer.");

    if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
        _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
      init(n_processors());
    else
      init(base_size);

    initialSetup();
  }
  SamplerTransientMultiApp::preTransfer(dt, target_time);
}

bool
PODTransientMultiApp::solveStep(Real dt, Real target_time, bool auto_advance)
{
  bool last_solve_converged = true;
  // If snapshot generation phase, solve the subapplications in a regular manner.
  // Otherwise, compute the residuals only.
  if (_fe_problem.getCurrentExecuteOnFlag() == StochasticTools::EXEC_POST_SNAPSHOT_GEN)
  {
    if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
        _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
      computeResidualBatch(target_time);
    else
      computeResidual();
  }
  else
  {
    last_solve_converged = SamplerTransientMultiApp::solveStep(dt, target_time, auto_advance);
  }

  return last_solve_converged;
}

void
PODTransientMultiApp::computeResidual()
{
  mooseError("Needs to be refactored based on the handling of the time-derivative.");
  // // Doing the regular computation but instead of solving the subapplication,
  // // the residuals for different tags are evaluated.
  // if (!_has_an_app)
  //   return;
  //
  // Moose::ScopedCommSwapper swapper(_my_comm);
  //
  // int rank;
  // int ierr;
  // ierr = MPI_Comm_rank(_communicator.get(), &rank);
  // mooseCheckMPIErr(ierr);
  //
  // // Getting the necessary tag names.
  // const std::vector<std::string> & trainer_tags = _trainer.getTagNames();
  //
  // // Looping over the subapplications and computing residuals.
  // for (unsigned int i = 0; i < _my_num_apps; i++)
  // {
  //   // Getting the local problem
  //   FEProblemBase & problem = _apps[i]->getExecutioner()->feProblem();
  //
  //   // Extracting the TagIDs based on tag names from the current subapp.
  //   std::set<TagID> tags_to_compute;
  //   for (auto & tag_name : trainer_tags)
  //     tags_to_compute.insert(problem.getVectorTagID(tag_name));
  //
  //   problem.computeResidualTags(tags_to_compute);
  // }
}

void
PODTransientMultiApp::computeResidualBatch(Real /*target_time*/)
{
  mooseError("Needs to be refactored based on the handling of the time-derivative.");
//   // Getting the overall base size from the trainer.
//   dof_id_type base_size = _trainer.getSumBaseSize();
//
//   // Distributing the residual evaluation among processes.
//   dof_id_type local_base_begin;
//   dof_id_type local_base_end;
//   dof_id_type n_local_bases;
//   MooseUtils::linearPartitionItems(
//       base_size, n_processors(), processor_id(), n_local_bases, local_base_begin, local_base_end);
//
//   // List of active relevant Transfer objects
//   std::vector<std::shared_ptr<PODSamplerSolutionTransfer>> to_transfers =
//       getActiveSolutionTransfers(MultiAppTransfer::TO_MULTIAPP);
//   std::vector<std::shared_ptr<PODResidualTransfer>> from_transfers =
//       getActiveResidualTransfers(MultiAppTransfer::FROM_MULTIAPP);
//
//   // Initialize to/from transfers
//   for (auto transfer : to_transfers)
//     transfer->initializeToMultiapp();
//
//   for (auto transfer : from_transfers)
//     transfer->initializeFromMultiapp();
//
//   if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
//     backup();
//
//   // Perform batch MultiApp solves
//   _local_batch_app_index = 0;
//   for (dof_id_type i = local_base_begin; i < local_base_end; ++i)
//   {
//     for (auto & transfer : to_transfers)
//     {
//       transfer->setGlobalMultiAppIndex(i);
//       transfer->executeToMultiapp();
//     }
//
//     computeResidual();
//
//     for (auto & transfer : from_transfers)
//     {
//       transfer->setGlobalMultiAppIndex(i);
//       transfer->executeFromMultiapp();
//     }
//
//     if (i < _sampler.getLocalRowEnd() - 1)
//     {
//       if (_mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
//         restore();
//       else
//       {
//         // The app is being reset for the next loop, thus the batch index must be indexed as such
//         _local_batch_app_index = i + 1;
//         resetApp(_local_batch_app_index, target_time);
//         initialSetup();
//       }
//     }
//   }
//   // Finalize to/from transfers
//   for (auto transfer : to_transfers)
//     transfer->finalizeToMultiapp();
//   for (auto transfer : from_transfers)
//     transfer->finalizeFromMultiapp();
}

std::vector<std::shared_ptr<PODSamplerSolutionTransfer>>
PODTransientMultiApp::getActiveSolutionTransfers(Transfer::DIRECTION direction)
{
  std::vector<std::shared_ptr<PODSamplerSolutionTransfer>> output;
  const ExecuteMooseObjectWarehouse<Transfer> & warehouse =
      _fe_problem.getMultiAppTransferWarehouse(direction);
  for (std::shared_ptr<Transfer> transfer : warehouse.getActiveObjects())
  {
    auto ptr = std::dynamic_pointer_cast<PODSamplerSolutionTransfer>(transfer);
    if (ptr)
      output.push_back(ptr);
  }
  return output;
}

std::vector<std::shared_ptr<PODResidualTransfer>>
PODTransientMultiApp::getActiveResidualTransfers(Transfer::DIRECTION direction)
{
  std::vector<std::shared_ptr<PODResidualTransfer>> output;
  const ExecuteMooseObjectWarehouse<Transfer> & warehouse =
      _fe_problem.getMultiAppTransferWarehouse(direction);
  for (std::shared_ptr<Transfer> transfer : warehouse.getActiveObjects())
  {
    auto ptr = std::dynamic_pointer_cast<PODResidualTransfer>(transfer);
    if (ptr)
      output.push_back(ptr);
  }
  return output;
}
