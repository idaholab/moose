//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationFullSolveMultiApp.h"
#include "OptimizationTransfer.h"

#include "Executioner.h"
#include "FEProblemBase.h"

registerMooseObject("StochasticToolsApp", OptimizationFullSolveMultiApp);

InputParameters
OptimizationFullSolveMultiApp::validParams()
{
  InputParameters params = FullSolveMultiApp::validParams();
  params.addClassDescription("Creates a full-solve type sub-application for Optimization object.");
  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<Point>>("move_positions");
  params.suppressParameter<std::vector<unsigned int>>("move_apps");
  return params;
}

OptimizationFullSolveMultiApp::OptimizationFullSolveMultiApp(const InputParameters & parameters)
  : FullSolveMultiApp(parameters),
    _perf_solve_optimization_step(registerTimedSection("solveStep", 1))
{
  size_t numSubApps = 2;
  init(numSubApps);
}

void
OptimizationFullSolveMultiApp::initialSetup()
{
  // fixme lynn copy and paste from fullSolveMultiApp because list of executioners are private.
  // could make these more specific to adjoint and forward subapps.
  MultiApp::initialSetup();

  if (_has_an_app)
  {
    Moose::ScopedCommSwapper swapper(_my_comm);

    _executioners.resize(_my_num_apps);

    // Grab Executioner from each app
    for (unsigned int i = 0; i < _my_num_apps; i++)
    {
      auto & app = _apps[i];
      Executioner * ex = app->getExecutioner();

      if (!ex)
        mooseError("Executioner does not exist!");

      ex->init();

      _executioners[i] = ex;
    }
  }
}

bool
OptimizationFullSolveMultiApp::solveStep(Real /*dt*/, Real /*target_time*/, bool auto_advance)
{
  TIME_SECTION(_perf_solve_optimization_step);

  // from fullsaolvemultiapp
  if (!auto_advance)
    mooseError("FullSolveMultiApp is not compatible with auto_advance=false");

  if (!_has_an_app)
    return true;

  Moose::ScopedCommSwapper swapper(_my_comm);

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_communicator.get(), &rank);
  mooseCheckMPIErr(ierr);

  // List of active relevant Transfer objects
  std::vector<std::shared_ptr<OptimizationTransfer>> to_transfers =
      getActiveOptimizationTransfers(MultiAppTransfer::TO_MULTIAPP);

  // Value to return
  bool last_solve_converged = true;

  size_t forwardAppIndex = 0;
  size_t adjointAppIndex = 1;

  to_transfers[0]->executeToMultiapp();
  last_solve_converged = executeAppSolve(forwardAppIndex);

  to_transfers[0]->executeToMultiapp();
  last_solve_converged = executeAppSolve(adjointAppIndex);

  to_transfers[0]->executeToMultiapp();
  last_solve_converged = executeAppSolve(forwardAppIndex);

  to_transfers[0]->executeToMultiapp();
  last_solve_converged = executeAppSolve(adjointAppIndex);
  return last_solve_converged;
}

bool
OptimizationFullSolveMultiApp::executeAppSolve(size_t appIndex)
{
  bool last_solve_converged = true;

  _apps[appIndex]->getOutputWarehouse().reset();
  Executioner * ex = _executioners[appIndex];
  ex->execute();
  if (!ex->lastSolveConverged())
    last_solve_converged = false;

  return last_solve_converged;
}

std::vector<std::shared_ptr<OptimizationTransfer>>
OptimizationFullSolveMultiApp::getActiveOptimizationTransfers(Transfer::DIRECTION direction)
{
  std::vector<std::shared_ptr<OptimizationTransfer>> output;
  const ExecuteMooseObjectWarehouse<Transfer> & warehouse =
      _fe_problem.getMultiAppTransferWarehouse(direction);
  for (std::shared_ptr<Transfer> transfer : warehouse.getActiveObjects())
  {
    auto ptr = std::dynamic_pointer_cast<OptimizationTransfer>(transfer);
    if (ptr)
      output.push_back(ptr);
  }
  return output;
}
