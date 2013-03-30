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
#include "TransientMultiApp.h"

#include "LayeredSideFluxAverage.h"

// libMesh
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<TransientMultiApp>()
{
  InputParameters params = validParams<MultiApp>();
  params += validParams<TransientInterface>();
  return params;
}


TransientMultiApp::TransientMultiApp(const std::string & name, InputParameters parameters):
    MultiApp(name, parameters),
    TransientInterface(parameters, name, "multiapps")
{
  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  _transient_executioners.resize(_my_num_apps);
  // Grab Transient Executioners from each app
  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    MooseApp * app = _apps[i];
    Transient * ex = dynamic_cast<Transient *>(app->getExecutioner());
    if(!ex)
      mooseError("MultiApp " << name << " is not using a Transient Executioner!");
    appProblem(_first_local_app + i)->initialSetup();
    ex->preExecute();
    appProblem(_first_local_app + i)->copyOldSolutions();
    _transient_executioners[i] = ex;
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

TransientMultiApp::~TransientMultiApp()
{
  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Transient * ex = _transient_executioners[i];
    ex->postExecute();
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

void
TransientMultiApp::solveStep()
{
  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  int rank;
  MPI_Comm_rank(_orig_comm, &rank);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Transient * ex = _transient_executioners[i];
    ex->takeStep(_dt);
    if(!ex->lastSolveConverged())
      mooseWarning(_name<<_first_local_app+i<<" failed to converge!"<<std::endl);
    ex->endStep();
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

Real
TransientMultiApp::computeDT()
{
  std::cout<<"computeDT!!!"<<std::endl;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  Real smallest_dt = std::numeric_limits<Real>::max();

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Transient * ex = _transient_executioners[i];
    Real dt = ex->computeConstrainedDT();

    std::cout<<"dt! "<<dt<<std::endl;

    smallest_dt = std::min(dt, smallest_dt);
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);

  Parallel::min(smallest_dt);

  std::cout<<"Smallest dt: "<<smallest_dt<<std::endl;
  return smallest_dt;
}

