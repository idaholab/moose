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

  params.addParam<bool>("sub_cycling", false, "Set to true to allow this MultiApp to take smaller timesteps than the rest of the simulation.  More than one timestep will be performed for each 'master' timestep");

  return params;
}


TransientMultiApp::TransientMultiApp(const std::string & name, InputParameters parameters):
    MultiApp(name, parameters),
    TransientInterface(parameters, name, "multiapps"),
    _sub_cycling(getParam<bool>("sub_cycling"))
{
  if(!_has_an_app)
    return;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  if(_has_an_app)
  {

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
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);
}

TransientMultiApp::~TransientMultiApp()
{
  if(!_has_an_app)
    return;

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
  if(!_has_an_app)
    return;

  std::cout<<"Solving MultiApp "<<_name<<std::endl;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  int rank;
  MPI_Comm_rank(_orig_comm, &rank);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Transient * ex = _transient_executioners[i];

    if(_sub_cycling)
    {
      // Get the dt this app wants to take
      Real dt = ex->computeConstrainedDT();

      // Divide the "master" dt by that
      Real partial_steps = _dt / dt;

      unsigned int num_steps = 0;

      if(partial_steps-std::floor(partial_steps) <= 2.0e-14)
        num_steps = std::floor(partial_steps);
      else
        num_steps = std::ceil(partial_steps);

      // Split the master dt up into the number of steps (so we can hit the time perfectly)
      dt = _dt / (Real)num_steps;

      // Now do all of the solves we need
      for(unsigned int i=0; i<num_steps; i++)
      {
        ex->takeStep(dt);

        if(!ex->lastSolveConverged())
          mooseWarning("While sub_cycling "<<_name<<_first_local_app+i<<" failed to converge!"<<std::endl);

        ex->endStep();
      }
    }
    else
    {
      ex->takeStep(_dt);

      if(!ex->lastSolveConverged())
        mooseWarning(_name<<_first_local_app+i<<" failed to converge!"<<std::endl);
      ex->endStep();
    }
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);

  std::cout<<"Finished Solving MultiApp "<<_name<<std::endl;
}

Real
TransientMultiApp::computeDT()
{
  if(_sub_cycling) // Bow out of the timestep selection dance
    return std::numeric_limits<Real>::max();

  Real smallest_dt = std::numeric_limits<Real>::max();

  if(_has_an_app)
  {
    MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

    for(unsigned int i=0; i<_my_num_apps; i++)
    {
      Transient * ex = _transient_executioners[i];
      Real dt = ex->computeConstrainedDT();

      smallest_dt = std::min(dt, smallest_dt);
    }

    // Swap back
    Moose::swapLibMeshComm(swapped);
  }

  Parallel::min(smallest_dt);
  return smallest_dt;
}

