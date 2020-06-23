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
#include "Sampler.h"
#include "StochasticToolsTransfer.h"

registerMooseObject("StochasticToolsApp", PODFullSolveMultiApp);

InputParameters
PODFullSolveMultiApp::validParams()
{
  InputParameters params = SamplerFullSolveMultiApp::validParams();
  params.addClassDescription(
      "Creates a full-solve type sub-application for each row of each Sampler matrix. "
      "Additionally, this runs subapplications with artificial solutions at final time.");
  params.addRequiredParam<std::string>("trainer_name", "Trainer object that contains the solutions for different samples.");

  return params;
}

PODFullSolveMultiApp::PODFullSolveMultiApp(const InputParameters & parameters)
  :
  SamplerFullSolveMultiApp(parameters),
  _trainer_name(getParam<std::string>("trainer_name"))
{
  if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
      _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    init(n_processors());
  else
    init(_sampler.getNumberOfRows());

  // Getting a pointer to the requested trainer object
  std::vector<PODReducedBasisTrainer *> obj;

  _fe_problem.theWarehouse()
               .query()
               .condition<AttribName>(_trainer_name)
               .queryInto(obj);

  if (obj.empty())
    mooseError("Unable to find Trainer with name '"+ _trainer_name + "'!");

  _trainer = obj[0];
}

bool
PODFullSolveMultiApp::solveStep(Real dt, Real target_time, bool auto_advance)
{
  bool last_solve_converged = true;

  const ExecFlagType& execute = _fe_problem.getCurrentExecuteOnFlag();

  std::cout << _fe_problem.getCurrentExecuteOnFlag() << std::endl;
  if (execute == EXEC_FINAL)
  {
    if (_mode == StochasticTools::MultiAppMode::BATCH_RESET ||
        _mode == StochasticTools::MultiAppMode::BATCH_RESTORE)
    {
        computeResidualBatch();
    }
    else
    {
        computeResidual();
    }
  }
  else
  {
    last_solve_converged = SamplerFullSolveMultiApp::solveStep(dt, target_time, auto_advance);
  }

  return last_solve_converged;
}

void
PODFullSolveMultiApp::preTransfer(Real dt, Real target_time)
{
  const ExecFlagType& execute = _fe_problem.getCurrentExecuteOnFlag();
  if (execute == EXEC_FINAL)
    init(_trainer->getSumBaseSize());

  SamplerFullSolveMultiApp::preTransfer(dt, target_time);
}

void
PODFullSolveMultiApp::computeResidual()
{

  Moose::ScopedCommSwapper swapper(_my_comm);

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_communicator.get(), &rank);
  mooseCheckMPIErr(ierr);

  for (unsigned int i = 0; i < _my_num_apps; i++)
  {
    std::cout << "Hello world" << std::endl;
  }
}
