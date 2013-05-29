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

#include "TimeStepper.h"
#include "LayeredSideFluxAverage.h"
#include "AllLocalDofIndicesThread.h"

// libMesh
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<TransientMultiApp>()
{
  InputParameters params = validParams<MultiApp>();
  params += validParams<TransientInterface>();

  params.addParam<bool>("sub_cycling", false, "Set to true to allow this MultiApp to take smaller timesteps than the rest of the simulation.  More than one timestep will be performed for each 'master' timestep");

  params.addParam<bool>("interpolate_transfers", false, "Only valid when sub_cycling.  This allows transferred values to be interpolated over the time frame the MultiApp is executing over when sub_cycling");

  params.addParam<bool>("detect_steady_state", false, "If true then while sub_cycling a steady state check will be done.  In this mode output will only be done once the MultiApp reaches the target time or steady state is reached");

  params.addParam<Real>("steady_state_tol", 1e-8, "The relative difference between the new solution and the old solution that will be considered to be at steady state");

  params.addParam<bool>("output_sub_cycles", false, "If true when sub_cycling every sub-cycle will be output.");

  params.addParam<unsigned int>("max_failures", 0, "Maximum number of solve failures tolerated while sub_cycling.");

  params.addParam<bool>("tolerate_failure", false, "If true this MultiApp won't participate in dt decisions and will always be fast-forwarded to the current time.");

  params.addParam<bool>("catch_up", false, "If true this will allow failed solves to attempt to 'catch up' using smaller timesteps.");

  params.addParam<Real>("max_catch_up_steps", 2, "Maximum number of steps to allow an app to take when trying to catch back up after a failed solve.");

  return params;
}


TransientMultiApp::TransientMultiApp(const std::string & name, InputParameters parameters):
    MultiApp(name, parameters),
    _sub_cycling(getParam<bool>("sub_cycling")),
    _interpolate_transfers(getParam<bool>("interpolate_transfers")),
    _detect_steady_state(getParam<bool>("detect_steady_state")),
    _steady_state_tol(getParam<Real>("steady_state_tol")),
    _output_sub_cycles(getParam<bool>("output_sub_cycles")),
    _max_failures(getParam<unsigned int>("max_failures")),
    _tolerate_failure(getParam<bool>("tolerate_failure")),
    _failures(0),
    _catch_up(getParam<bool>("catch_up")),
    _max_catch_up_steps(getParam<Real>("max_catch_up_steps"))
{
  // Transfer interpolation only makes sense for sub-cycling solves
  if(_interpolate_transfers && !_sub_cycling)
    mooseError("MultiApp " << _name << " is set to interpolate_transfers but is not sub_cycling!  That is not valid!");

  if(!_has_an_app)
    return;

  MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

  if(_has_an_app)
  {
    _transient_executioners.resize(_my_num_apps);
    // Grab Transient Executioners from each app
    for(unsigned int i=0; i<_my_num_apps; i++)
      setupApp(i);
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

NumericVector<Number> &
TransientMultiApp::appTransferVector(unsigned int app, std::string var_name)
{
  if(std::find(_transferred_vars.begin(), _transferred_vars.end(), var_name) == _transferred_vars.end())
    _transferred_vars.push_back(var_name);

  if(_interpolate_transfers)
    return appProblem(app)->getAuxiliarySystem().system().get_vector("transfer");

  return appProblem(app)->getAuxiliarySystem().solution();
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
      Real time_old = ex->getTime();

      if(_interpolate_transfers)
      {
        FEProblem * problem = appProblem(_first_local_app + i);
        AuxiliarySystem & aux_system = problem->getAuxiliarySystem();
        System & libmesh_aux_system = aux_system.system();

        NumericVector<Number> & solution = *libmesh_aux_system.solution;
        NumericVector<Number> & transfer_old = libmesh_aux_system.get_vector("transfer_old");

        solution.close();

        // Save off the current auxiliary solution
        transfer_old = solution;

        transfer_old.close();

        // Snag all of the local dof indices for all of these variables
        AllLocalDofIndicesThread aldit(libmesh_aux_system, _transferred_vars);
        ConstElemRange & elem_range = *problem->mesh().getActiveLocalElementRange();
        Threads::parallel_reduce(elem_range, aldit);

        _transferred_dofs = aldit._all_dof_indices;
      }

      if(_output_sub_cycles)
        ex->allowOutput(true);
      else
        ex->allowOutput(false);

      ex->setTargetTime(target_time);

//      unsigned int failures = 0;

      bool at_steady = false;

      // Now do all of the solves we need
      while(!at_steady && ex->getTime() + 2e-14 < target_time)
      {
        ex->getTimeStepper()->preStep();

        if(_interpolate_transfers)
        {
          // See what time this executioner is going to go to.
          Real future_time = ex->getTime() + ex->computeConstrainedDT();

          // How far along we are towards the target time:
          Real step_percent = (future_time - time_old) / (target_time - time_old);

          Real one_minus_step_percent = 1.0 - step_percent;

          // Do the interpolation for each variable that was transferred to
          FEProblem * problem = appProblem(_first_local_app + i);
          AuxiliarySystem & aux_system = problem->getAuxiliarySystem();
          System & libmesh_aux_system = aux_system.system();

          NumericVector<Number> & solution = *libmesh_aux_system.solution;
          NumericVector<Number> & transfer = libmesh_aux_system.get_vector("transfer");
          NumericVector<Number> & transfer_old = libmesh_aux_system.get_vector("transfer_old");

          solution.close(); // Just to be sure
          transfer.close();
          transfer_old.close();

          std::set<unsigned int>::iterator it  = _transferred_dofs.begin();
          std::set<unsigned int>::iterator end = _transferred_dofs.end();

          for(; it != end; ++it)
          {
            unsigned int dof = *it;
            solution.set(dof, (transfer_old(dof) * one_minus_step_percent) + (transfer(dof) * step_percent));
//            solution.set(dof, transfer_old(dof));
//            solution.set(dof, transfer(dof));
//            solution.set(dof, 1);
          }

          solution.close();
        }

        ex->takeStep();

        bool converged = ex->lastSolveConverged();

        if(!converged)
        {
          mooseWarning("While sub_cycling "<<_name<<_first_local_app+i<<" failed to converge!"<<std::endl);
          _failures++;

          if(_failures > _max_failures)
            mooseError("While sub_cycling "<<_name<<_first_local_app+i<<" REALLY failed!"<<std::endl);
        }

        Real solution_change_norm = ex->getSolutionChangeNorm();

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
      std::cout<<"Solving Normal Step!"<<std::endl;

      // ex->getTimeStepper()->preStep() was already called by computeDT()!
      ex->takeStep(dt);
      ex->endStep();

      if(!ex->lastSolveConverged())
      {
        mooseWarning(_name<<_first_local_app+i<<" failed to converge!"<<std::endl);

        if(_catch_up)
        {
          std::cout<<"Starting Catch Up!"<<std::endl;

          bool caught_up = false;

          unsigned int catch_up_step = 0;

          Real catch_up_dt = dt/2;

          ex->allowOutput(false); // Don't output while catching up

          while(!caught_up && catch_up_step < _max_catch_up_steps)
          {
            std::cerr<<"Solving " << _name << "catch up step " << catch_up_step <<std::endl;

            ex->getTimeStepper()->preStep();
            ex->computeConstrainedDT(); // Have to call this even though we're not using the dt it computes
            ex->takeStep(catch_up_dt); // Cut the timestep in half to try two half-step solves

            if(ex->lastSolveConverged())
            {
              if(ex->getTime() + 2e-14 >= target_time)
              {
                ex->forceOutput(); // This is here so that it is called before endStep()
                caught_up = true;
              }
            }
            else
              catch_up_dt /= 2.0;

            ex->endStep(); // This is here so it is called after forceOutput()

            catch_up_step++;
          }

          if(!caught_up)
            mooseError(_name << " Failed to catch up!" << std::endl);

          ex->allowOutput(true);
        }
      }
    }
  }

  // Swap back
  Moose::swapLibMeshComm(swapped);

  _transferred_vars.clear();

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
      ex->getTimeStepper()->preStep();
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

void
TransientMultiApp::resetApp(unsigned int global_app, Real /*time*/)  // FIXME: Note that we are passing in time but also grabbing it below
{
  if(hasLocalApp(global_app))
  {
    unsigned int local_app = globalAppToLocal(global_app);

    // Grab the current time the App is at so we can start the new one at the same place
    Real time = _transient_executioners[local_app]->getTime();

    MultiApp::resetApp(global_app, time);

    MPI_Comm swapped = Moose::swapLibMeshComm(_my_comm);

    setupApp(local_app, time, false);

    // Swap back
    Moose::swapLibMeshComm(swapped);
  }
}

void
TransientMultiApp::setupApp(unsigned int i, Real /*time*/, bool output_initial)  // FIXME: Should we be passing time?
{
  MooseApp * app = _apps[i];
  Transient * ex = dynamic_cast<Transient *>(app->getExecutioner());
  if(!ex)
    mooseError("MultiApp " << _name << " is not using a Transient Executioner!");

  if(!output_initial)
    ex->outputInitial(false);

  ex->init();

  FEProblem * problem = appProblem(_first_local_app + i);

  if(_interpolate_transfers)
  {
    AuxiliarySystem & aux_system = problem->getAuxiliarySystem();
    System & libmesh_aux_system = aux_system.system();

    // We'll store a copy of the auxiliary system's solution at the old time in here
    libmesh_aux_system.add_vector("transfer_old", false);

    // This will be where we'll transfer the value to for the "target" time
    libmesh_aux_system.add_vector("transfer", false);
  }

  ex->preExecute();
  problem->copyOldSolutions();
  _transient_executioners[i] = ex;

  if(_detect_steady_state || _tolerate_failure)
    ex->allowOutput(false);
}

