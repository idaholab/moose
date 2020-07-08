//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "PODFullSolveMultiApp.h"
#include "NonlinearSystemBase.h"
#include "Sampler.h"
#include "Executioner.h"

registerMooseObject("StochasticToolsApp", PODFullSolveMultiApp);

InputParameters
PODFullSolveMultiApp::validParams()
{
  InputParameters params = SamplerFullSolveMultiApp::validParams();
  params.addClassDescription(
      "Creates a full-solve type sub-application for each row of each Sampler matrix. "
      "Additionally, this runs subapplications with artificial solutions at final time.");
  params.addRequiredParam<std::string>(
      "trainer_name", "Trainer object that contains the solutions for different samples.");

  return params;
}

PODFullSolveMultiApp::PODFullSolveMultiApp(const InputParameters & parameters)
  : SamplerFullSolveMultiApp(parameters),
    _trainer_name(getParam<std::string>("trainer_name")),
    _snapshot_generation(true)
{
  // Initializing the subapps
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
      _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    init(n_processors());
  else
    init(_sampler.getNumberOfRows());

  // Getting a pointer to the requested trainer object
  std::vector<PODReducedBasisTrainer *> obj;

  _fe_problem.theWarehouse().query().condition<AttribName>(_trainer_name).queryInto(obj);

  if (obj.empty())
    paramError("trainer_name", "Unable to find Trainer with name '" + _trainer_name + "'!");

  _trainer = obj[0];
}

void
PODFullSolveMultiApp::preTransfer(Real dt, Real target_time)
{
  // Reinitialize the problem only if the snapshot generation part is done.
  if (!_snapshot_generation)
  {
    // Since it only works in serial, the number of processes is hardcoded to 1
    if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
        _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
      init(1);
    else
      init(_trainer->getSumBaseSize());

    initialSetup();
  }
  SamplerFullSolveMultiApp::preTransfer(dt, target_time);
}

bool
PODFullSolveMultiApp::solveStep(Real dt, Real target_time, bool auto_advance)
{
  bool last_solve_converged = true;

  // If snapshot generation phase, solve the subapplications in a regular manner.
  // Otherwise, compute the residuals only.
  if (_snapshot_generation)
  {
    last_solve_converged = SamplerFullSolveMultiApp::solveStep(dt, target_time, auto_advance);
    _snapshot_generation = false;
  }
  else
  {
    if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
        _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
      computeResidualBatch(target_time);
    else
      computeResidual();
  }

  return last_solve_converged;
}

void
PODFullSolveMultiApp::computeResidual()
{
  // Doing the regual computation but instead of solving the subapplication,
  // the residuals for different tags are evaluated.
  if (!_has_an_app)
    return;

  Moose::ScopedCommSwapper swapper(_my_comm);

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_communicator.get(), &rank);
  mooseCheckMPIErr(ierr);

  // Getting the necessary tag names.
  const std::vector<std::string> & trainer_tags = _trainer->getTagNames();

  // Looping over the subapplications and computing residuals.
  for (unsigned int i = 0; i < _my_num_apps; i++)
  {
    // Getting the local problem
    FEProblemBase & problem = _apps[i]->getExecutioner()->feProblem();

    // Extracting the TagIDs based on tag names from the current subapp.
    std::set<TagID> tags_to_compute;
    for (auto & tag_name : trainer_tags)
      tags_to_compute.insert(problem.getVectorTagID(tag_name));

    problem.computeResidualTags(tags_to_compute);
  }
}

void PODFullSolveMultiApp::computeResidualBatch(Real /*target_time*/)
{
  mooseError("Batch mode is not implemented yet for PODFullSolveMultiApp!");
}
