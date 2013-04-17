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

  params.addParam<bool>("detect_steady_state", false, "If true then while sub_cycling a steady state check will be done.  In this mode output will only be done once the MultiApp reaches the target time or steady state is reached");

  params.addParam<Real>("steady_state_tol", 1e-8, "The relative difference between the new solution and the old solution that will be considered to be at steady state");

  params.addParam<unsigned int>("max_failures", 0, "Maximum number of solve failures tolerated while sub_cycling.");


  return params;
}


TransientMultiApp::TransientMultiApp(const std::string & name, InputParameters parameters):
    MultiApp(name, parameters),
    TransientInterface(parameters, name, "multiapps"),
    _sub_cycling(getParam<bool>("sub_cycling")),
    _detect_steady_state(getParam<bool>("detect_steady_state")),
    _steady_state_tol(getParam<Real>("steady_state_tol")),
    _max_failures(getParam<unsigned int>("max_failures")),
    _failures(0)
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

      if(_detect_steady_state)
        ex->allowOutput(false);
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
      ex->setTargetTime(_t);

      unsigned int failures = 0;

      bool at_steady = false;

      // Now do all of the solves we need
      while(!at_steady && ex->getTime() + 2e-14 < _t)
      {
        ex->takeStep();

        bool converged = ex->lastSolveConverged();

        if(!converged)
        {
          mooseWarning("While sub_cycling "<<_name<<_first_local_app+i<<" failed to converge!"<<std::endl);
          _failures++;

          if(_failures > _max_failures)
            mooseError("While sub_cycling "<<_name<<_first_local_app+i<<" REALLY failed!"<<std::endl);
        }

        Real solution_change_norm = ex->solutionChangeNorm();

        if(_detect_steady_state)
          std::cout<<"Solution change norm: "<<solution_change_norm<<std::endl;

        if(converged && _detect_steady_state && solution_change_norm < _steady_state_tol)
        {
          std::cout<<"Detected Steady State!  Fast-forwarding to "<<_t<<std::endl;

          at_steady = true;

          // Set the time for the problem to the target time we were looking for
          ex->setTime(_t);

          // Force it to output right now
          ex->forceOutput();

          // Clean up the end
          ex->endStep();
        }
        else
          ex->endStep();
      }

      // If we were looking for a steady state, but didn't reach one, we still need to output one more time
      if(_detect_steady_state && !at_steady)
        ex->forceOutput();
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

