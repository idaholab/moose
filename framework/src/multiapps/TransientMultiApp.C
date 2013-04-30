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

  params.addParam<bool>("tolerate_failure", false, "If true this MultiApp won't participate in dt decisions and will always be fast-forwarded to the current time.");

  params.addParam<bool>("catch_up", false, "If true this will allow failed solves to attempt to 'catch up' using smaller timesteps.");


  return params;
}


TransientMultiApp::TransientMultiApp(const std::string & name, InputParameters parameters):
    MultiApp(name, parameters),
    _sub_cycling(getParam<bool>("sub_cycling")),
    _detect_steady_state(getParam<bool>("detect_steady_state")),
    _steady_state_tol(getParam<Real>("steady_state_tol")),
    _max_failures(getParam<unsigned int>("max_failures")),
    _tolerate_failure(getParam<bool>("tolerate_failure")),
    _failures(0),
    _catch_up(false)
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

      if(_detect_steady_state || _tolerate_failure)
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
TransientMultiApp::solveStep(Real dt, Real target_time)
{
  if(!_has_an_app)
    return;

  std::cout<<"Solving MultiApp "<<_name<<std::endl;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_orig_comm, &rank); mooseCheckMPIErr(ierr);

  for(unsigned int i=0; i<_my_num_apps; i++)
  {
    Transient * ex = _transient_executioners[i];

    if(ex->getTime() + 2e-14 >= target_time) // Maybe this MultiApp was already solved
      continue;

    if(_sub_cycling)
    {
      ex->allowOutput(false); // Don't Output
      ex->setTargetTime(target_time);

      unsigned int failures = 0;

      bool at_steady = false;

      // Now do all of the solves we need
      while(!at_steady && ex->getTime() + 2e-14 < target_time)
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
          std::cout<<"Detected Steady State!  Fast-forwarding to "<<target_time<<std::endl;

          at_steady = true;

          // Set the time for the problem to the target time we were looking for
          ex->setTime(target_time);

          // Force it to output right now
          ex->forceOutput();

          // Clean up the end
          ex->endStep();
        }
        else
          ex->endStep();
      }

      // If we were looking for a steady state, but didn't reach one, we still need to output one more time
      if(!at_steady)
        ex->forceOutput();
    }
    else if(_tolerate_failure)
    {
      ex->computeConstrainedDT();
      ex->takeStep(dt);
      ex->setTime(target_time);
      ex->forceOutput();
      ex->endStep();
    }
    else
    {
      ex->takeStep(dt);
      ex->endStep();

      if(!ex->lastSolveConverged())
      {
        mooseWarning(_name<<_first_local_app+i<<" failed to converge!"<<std::endl);

        if(_catch_up)
        {
          ex->allowOutput(false); // Don't output while catching up
          ex->computeConstrainedDT(); // Have to call this even though we're not using the dt it computes
          ex->takeStep(dt / 2.0); // Cut the timestep in half to try two half-step solves
          ex->endStep();

          if(!ex->lastSolveConverged())
            mooseError(_name<<_first_local_app+i<<" failed to converge during first catch up!"<<std::endl);

          ex->allowOutput(true);

          ex->computeConstrainedDT();
          ex->takeStep(dt / 2.0);
          ex->endStep();

          if(!ex->lastSolveConverged())
            mooseError(_name<<_first_local_app+i<<" failed to converge during second catch up!"<<std::endl);
        }
      }
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

  if(_tolerate_failure) // Bow out of the timestep selection dance, we do this down here because we need to call computeConstrainedDT at least once for these executioners...
    return std::numeric_limits<Real>::max();


  Parallel::min(smallest_dt);
  return smallest_dt;
}

