/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FullSolveMultiApp.h"
#include "LayeredSideFluxAverage.h"
#include "Executioner.h"

// libMesh
#include "libmesh/mesh_tools.h"

template <>
InputParameters
validParams<FullSolveMultiApp>()
{
  InputParameters params = validParams<MultiApp>();
  return params;
}

FullSolveMultiApp::FullSolveMultiApp(const InputParameters & parameters)
  : MultiApp(parameters), _solved(false)
{
}

void
FullSolveMultiApp::initialSetup()
{
  MultiApp::initialSetup();

  if (_has_an_app)
  {
    MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

    _executioners.resize(_my_num_apps);

    // Grab Executioner from each app
    for (unsigned int i = 0; i < _my_num_apps; i++)
    {
      MooseApp * app = _apps[i];
      Executioner * ex = app->getExecutioner();

      if (!ex)
        mooseError("Executioner does not exist!");

      ex->init();

      _executioners[i] = ex;
    }
    // Swap back
    Moose::swapLibMeshComm(swapped);
  }
}

bool
FullSolveMultiApp::solveStep(Real /*dt*/, Real /*target_time*/, bool auto_advance)
{
  if (!auto_advance)
    mooseError("FullSolveMultiApp is not compatible with auto_advance=false");

  if (!_has_an_app)
    return true;

  if (_solved)
    return true;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_orig_comm, &rank);
  mooseCheckMPIErr(ierr);

  bool last_solve_converged = true;
  for (unsigned int i = 0; i < _my_num_apps; i++)
  {
    Executioner * ex = _executioners[i];
    ex->execute();
    if (!ex->lastSolveConverged())
      last_solve_converged = false;
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);

  _solved = true;

  return last_solve_converged;
}
