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

// libMesh
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<FullSolveMultiApp>()
{
  InputParameters params = validParams<MultiApp>();
  return params;
}


FullSolveMultiApp::FullSolveMultiApp(const std::string & name, InputParameters parameters):
    MultiApp(name, parameters),
    _solved(false)
{
}

FullSolveMultiApp::~FullSolveMultiApp()
{
}

void
FullSolveMultiApp::init()
{
  MultiApp::init();

  if (_has_an_app)
  {
    MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

    _executioners.resize(_my_num_apps);

    // Grab Executioner from each app
    for(unsigned int i=0; i<_my_num_apps; i++)
    {
      MooseApp * app = _apps[i];
      Executioner * ex = app->getExecutioner();

      if (!ex)
        mooseError("Executioner does not exist!");

      _executioners[i] = ex;
    }
    // Swap back
    Moose::swapLibMeshComm(swapped);
  }
}

void
FullSolveMultiApp::solveStep(Real /*dt*/, Real /*target_time*/, bool auto_advance)
{
  if (!auto_advance)
    mooseError("FullSolveMultiApp is not compatible with auto_advance=false");

  if (!_has_an_app)
    return;

  if (_solved)
    return;

  Moose::out << "Fully Solving MultiApp " << _name << std::endl;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_orig_comm, &rank); mooseCheckMPIErr(ierr);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Executioner * ex = _executioners[i];
    ex->init();
    ex->execute();
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);

  _solved = true;

  Moose::out << "Finished Solving MultiApp " << _name << std::endl;
}
