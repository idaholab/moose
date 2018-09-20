//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FullSolveMultiApp.h"
#include "LayeredSideFluxAverage.h"
#include "Executioner.h"

// libMesh
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", FullSolveMultiApp);

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
FullSolveMultiApp::solveStep(Real /*dt*/, Real /*target_time*/, bool auto_advance)
{
  if (!auto_advance)
    mooseError("FullSolveMultiApp is not compatible with auto_advance=false");

  if (!_has_an_app)
    return true;

  if (_solved)
    return true;

  Moose::ScopedCommSwapper swapper(_my_comm);

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_communicator.get(), &rank);
  mooseCheckMPIErr(ierr);

  bool last_solve_converged = true;
  for (unsigned int i = 0; i < _my_num_apps; i++)
  {
    Executioner * ex = _executioners[i];
    ex->execute();
    if (!ex->lastSolveConverged())
      last_solve_converged = false;
  }

  _solved = true;

  return last_solve_converged;
}
