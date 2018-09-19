//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "TransientMultiApp.h"

#include "AllLocalDofIndicesThread.h"
#include "AuxiliarySystem.h"
#include "Console.h"
#include "LayeredSideFluxAverage.h"
#include "MooseMesh.h"
#include "Output.h"
#include "TimeStepper.h"
#include "Transient.h"
#include "NonlinearSystem.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/numeric_vector.h"

registerMooseObject("MooseApp", TransientMultiApp);

template <>
InputParameters
validParams<TransientMultiApp>()
{
  InputParameters params = validParams<MultiApp>();
  params += validParams<TransientInterface>();

  params.addParam<bool>("sub_cycling",
                        false,
                        "Set to true to allow this MultiApp to take smaller "
                        "timesteps than the rest of the simulation.  More "
                        "than one timestep will be performed for each "
                        "'master' timestep");

  params.addParam<bool>("interpolate_transfers",
                        false,
                        "Only valid when sub_cycling.  This allows "
                        "transferred values to be interpolated "
                        "over the time frame the MultiApp is "
                        "executing over when sub_cycling");

  params.addParam<bool>("detect_steady_state",
                        false,
                        "If true then while sub_cycling a steady state check will be "
                        "done.  In this mode output will only be done once the "
                        "MultiApp reaches the target time or steady state is reached");

  params.addParam<Real>("steady_state_tol",
                        1e-8,
                        "The relative difference between the new "
                        "solution and the old solution that will be "
                        "considered to be at steady state");

  params.addParam<bool>("output_sub_cycles", false, "If true then every sub-cycle will be output.");
  params.addParam<bool>(
      "print_sub_cycles", true, "Toggle the display of sub-cycles on the screen.");

  params.addParam<unsigned int>(
      "max_failures", 0, "Maximum number of solve failures tolerated while sub_cycling.");

  params.addParam<bool>("tolerate_failure",
                        false,
                        "If true this MultiApp won't participate in dt "
                        "decisions and will always be fast-forwarded to "
                        "the current time.");

  params.addParam<bool>(
      "catch_up",
      false,
      "If true this will allow failed solves to attempt to 'catch up' using smaller timesteps.");

  params.addParam<bool>("keep_solution_during_restore",
                        false,
                        "This is useful when doing Picard with catch_up steps.  It takes the "
                        "solution from the final catch_up step and re-uses it as the initial guess "
                        "for the next picard iteration");

  params.addParam<Real>("max_catch_up_steps",
                        2,
                        "Maximum number of steps to allow an app to take "
                        "when trying to catch back up after a failed "
                        "solve.");

  return params;
}

TransientMultiApp::TransientMultiApp(const InputParameters & parameters)
  : MultiApp(parameters),
    _sub_cycling(getParam<bool>("sub_cycling")),
    _interpolate_transfers(getParam<bool>("interpolate_transfers")),
    _detect_steady_state(getParam<bool>("detect_steady_state")),
    _steady_state_tol(getParam<Real>("steady_state_tol")),
    _output_sub_cycles(getParam<bool>("output_sub_cycles")),
    _max_failures(getParam<unsigned int>("max_failures")),
    _tolerate_failure(getParam<bool>("tolerate_failure")),
    _failures(0),
    _catch_up(getParam<bool>("catch_up")),
    _max_catch_up_steps(getParam<Real>("max_catch_up_steps")),
    _keep_solution_during_restore(getParam<bool>("keep_solution_during_restore")),
    _first(declareRecoverableData<bool>("first", true)),
    _auto_advance(false),
    _print_sub_cycles(getParam<bool>("print_sub_cycles"))
{
  // Transfer interpolation only makes sense for sub-cycling solves
  if (_interpolate_transfers && !_sub_cycling)
    mooseError("MultiApp ",
               name(),
               " is set to interpolate_transfers but is not sub_cycling!  That is not valid!");

  // Subcycling overrides catch up, we don't want to confuse users by allowing them to set both.
  if (_sub_cycling && _catch_up)
    mooseError("MultiApp ",
               name(),
               " sub_cycling and catch_up cannot both be set to true simultaneously.");

  if (_sub_cycling && _keep_solution_during_restore)
    mooseError("In MultiApp ",
               name(),
               " it doesn't make any sense to keep a solution during restore when doing "
               "sub_cycling.  Consider trying catch_up steps instead");

  if (!_catch_up && _keep_solution_during_restore)
    mooseError("In MultiApp ",
               name(),
               " `keep_solution_during_restore` requires `catch_up = true`.  Either disable "
               "`keep_solution_during_restart` or set `catch_up = true`");
}

NumericVector<Number> &
TransientMultiApp::appTransferVector(unsigned int app, std::string var_name)
{
  if (std::find(_transferred_vars.begin(), _transferred_vars.end(), var_name) ==
      _transferred_vars.end())
    _transferred_vars.push_back(var_name);

  if (_interpolate_transfers)
    return appProblemBase(app).getAuxiliarySystem().system().get_vector("transfer");

  return appProblemBase(app).getAuxiliarySystem().solution();
}

void
TransientMultiApp::initialSetup()
{
  MultiApp::initialSetup();

  if (!_has_an_app)
    return;

  Moose::ScopedCommSwapper swapper(_my_comm);

  if (_has_an_app)
  {
    _transient_executioners.resize(_my_num_apps);
    // Grab Transient Executioners from each app
    for (unsigned int i = 0; i < _my_num_apps; i++)
      setupApp(i);
  }
}

void
TransientMultiApp::restore()
{
  // Must be restarting / recovering so hold off on restoring
  // Instead - the restore will happen in createApp()
  // Note that _backups was already populated by dataLoad()
  if (_apps.empty())
    return;

  if (_keep_solution_during_restore)
  {
    _end_solutions.resize(_my_num_apps);

    for (unsigned int i = 0; i < _my_num_apps; i++)
      _end_solutions[i] =
          _apps[i]->getExecutioner()->feProblem().getNonlinearSystem().solution().clone();
  }

  MultiApp::restore();

  if (_keep_solution_during_restore)
  {
    for (unsigned int i = 0; i < _my_num_apps; i++)
      _apps[i]->getExecutioner()->feProblem().getNonlinearSystem().solution() = *_end_solutions[i];

    _end_solutions.clear();
  }
}

bool
TransientMultiApp::solveStep(Real dt, Real target_time, bool auto_advance)
{
  if (!_has_an_app)
    return true;

  _auto_advance = auto_advance;

  _console << "Solving MultiApp " << name() << std::endl;

  // "target_time" must always be in global time
  target_time += _app.getGlobalTimeOffset();

  Moose::ScopedCommSwapper swapper(_my_comm);
  bool return_value = true;

  // Make sure we swap back the communicator regardless of how this routine is exited
  try
  {
    int rank;
    int ierr;
    ierr = MPI_Comm_rank(_communicator.get(), &rank);
    mooseCheckMPIErr(ierr);

    for (unsigned int i = 0; i < _my_num_apps; i++)
    {

      FEProblemBase & problem = appProblemBase(_first_local_app + i);

      Transient * ex = _transient_executioners[i];

      // The App might have a different local time from the rest of the problem
      Real app_time_offset = _apps[i]->getGlobalTimeOffset();

      // Maybe this MultiApp was already solved
      if ((ex->getTime() + app_time_offset + 2e-14 >= target_time) ||
          (ex->getTime() >= ex->endTime()))
        continue;

      if (_sub_cycling)
      {
        Real time_old = ex->getTime() + app_time_offset;

        if (_interpolate_transfers)
        {
          AuxiliarySystem & aux_system = problem.getAuxiliarySystem();
          System & libmesh_aux_system = aux_system.system();

          NumericVector<Number> & solution = *libmesh_aux_system.solution;
          NumericVector<Number> & transfer_old = libmesh_aux_system.get_vector("transfer_old");

          solution.close();

          // Save off the current auxiliary solution
          transfer_old = solution;

          transfer_old.close();

          // Snag all of the local dof indices for all of these variables
          AllLocalDofIndicesThread aldit(libmesh_aux_system, _transferred_vars);
          ConstElemRange & elem_range = *problem.mesh().getActiveLocalElementRange();
          Threads::parallel_reduce(elem_range, aldit);

          _transferred_dofs = aldit._all_dof_indices;
        }

        // Disable/enable output for sub cycling
        problem.allowOutput(_output_sub_cycles);         // disables all outputs, including console
        problem.allowOutput<Console>(_print_sub_cycles); // re-enables Console to print, if desired

        ex->setTargetTime(target_time - app_time_offset);

        //      unsigned int failures = 0;

        bool at_steady = false;

        if (_first && !_app.isRecovering())
          problem.advanceState();

        bool local_first = _first;

        // Now do all of the solves we need
        while ((!at_steady && ex->getTime() + app_time_offset + 2e-14 < target_time) ||
               !ex->lastSolveConverged())
        {
          if (local_first != true)
            ex->incrementStepOrReject();

          local_first = false;

          ex->preStep();
          ex->computeDT();

          if (_interpolate_transfers)
          {
            // See what time this executioner is going to go to.
            Real future_time = ex->getTime() + app_time_offset + ex->getDT();

            // How far along we are towards the target time:
            Real step_percent = (future_time - time_old) / (target_time - time_old);

            Real one_minus_step_percent = 1.0 - step_percent;

            // Do the interpolation for each variable that was transferred to
            FEProblemBase & problem = appProblemBase(_first_local_app + i);
            AuxiliarySystem & aux_system = problem.getAuxiliarySystem();
            System & libmesh_aux_system = aux_system.system();

            NumericVector<Number> & solution = *libmesh_aux_system.solution;
            NumericVector<Number> & transfer = libmesh_aux_system.get_vector("transfer");
            NumericVector<Number> & transfer_old = libmesh_aux_system.get_vector("transfer_old");

            solution.close(); // Just to be sure
            transfer.close();
            transfer_old.close();

            for (const auto & dof : _transferred_dofs)
            {
              solution.set(dof,
                           (transfer_old(dof) * one_minus_step_percent) +
                               (transfer(dof) * step_percent));
              //            solution.set(dof, transfer_old(dof));
              //            solution.set(dof, transfer(dof));
              //            solution.set(dof, 1);
            }

            solution.close();
          }

          ex->takeStep();

          bool converged = ex->lastSolveConverged();

          if (!converged)
          {
            mooseWarning(
                "While sub_cycling ", name(), _first_local_app + i, " failed to converge!\n");

            _failures++;

            if (_failures > _max_failures)
            {
              std::stringstream oss;
              oss << "While sub_cycling " << name() << _first_local_app << i << " REALLY failed!";
              throw MultiAppSolveFailure(oss.str());
            }
          }

          Real solution_change_norm = ex->getSolutionChangeNorm();

          if (_detect_steady_state)
            _console << "Solution change norm: " << solution_change_norm << std::endl;

          if (converged && _detect_steady_state && solution_change_norm < _steady_state_tol)
          {
            _console << "Detected Steady State!  Fast-forwarding to " << target_time << std::endl;

            at_steady = true;

            // Indicate that the next output call (occurs in ex->endStep()) should output,
            // regardless of intervals etc...
            problem.forceOutput();

            // Clean up the end
            ex->endStep(target_time - app_time_offset);
            ex->postStep();
          }
          else
          {
            ex->endStep();
            ex->postStep();
          }
        }

        // If we were looking for a steady state, but didn't reach one, we still need to output one
        // more time, regardless of interval
        if (!at_steady)
          problem.outputStep(EXEC_FORCED);

      } // sub_cycling
      else if (_tolerate_failure)
      {
        ex->takeStep(dt);
        ex->endStep(target_time - app_time_offset);
        ex->postStep();
      }
      else
      {
        _console << "Solving Normal Step!" << std::endl;

        if (_first && !_app.isRecovering())
          problem.advanceState();

        if (auto_advance)
          problem.allowOutput(true);

        ex->takeStep(dt);

        if (auto_advance)
        {
          ex->endStep();
          ex->postStep();

          if (!ex->lastSolveConverged())
          {
            mooseWarning(name(), _first_local_app + i, " failed to converge!\n");

            if (_catch_up)
            {
              _console << "Starting Catch Up!" << std::endl;

              bool caught_up = false;

              unsigned int catch_up_step = 0;

              Real catch_up_dt = dt / 2;

              while (!caught_up && catch_up_step < _max_catch_up_steps)
              {
                _console << "Solving " << name() << " catch up step " << catch_up_step << std::endl;
                ex->incrementStepOrReject();

                ex->computeDT();
                ex->takeStep(catch_up_dt); // Cut the timestep in half to try two half-step solves
                ex->endStep();

                if (ex->lastSolveConverged())
                {
                  if (ex->getTime() + app_time_offset +
                          (ex->timestepTol() * std::abs(ex->getTime())) >=
                      target_time)
                  {
                    problem.outputStep(EXEC_FORCED);
                    caught_up = true;
                  }
                }
                else
                  catch_up_dt /= 2.0;

                ex->postStep();

                catch_up_step++;
              }

              if (!caught_up)
                throw MultiAppSolveFailure(name() + " Failed to catch up!\n");
            }
          }
        }
        else
        {
          if (!ex->lastSolveConverged())
          {
            // Even if we don't allow auto_advance - we can still catch up to the current time if
            // possible
            if (_catch_up)
            {
              _console << "Starting Catch Up!" << std::endl;

              bool caught_up = false;

              unsigned int catch_up_step = 0;

              Real catch_up_dt = dt / 2;

              // Note: this loop will _break_ if target_time is satisfied
              while (catch_up_step < _max_catch_up_steps)
              {
                _console << "Solving " << name() << " catch up step " << catch_up_step << std::endl;
                ex->incrementStepOrReject();

                ex->computeDT();
                ex->takeStep(catch_up_dt); // Cut the timestep in half to try two half-step solves

                // This is required because we can't call endStep() yet
                // (which normally increments time)
                Real current_time = ex->getTime() + ex->getDT();

                if (ex->lastSolveConverged())
                {
                  if (current_time + app_time_offset +
                          (ex->timestepTol() * std::abs(current_time)) >=
                      target_time)
                  {
                    caught_up = true;
                    break; // break here so that we don't run endStep() or postStep() since this
                           // MultiApp should NOT be auto_advanced
                  }
                }
                else
                  catch_up_dt /= 2.0;

                ex->endStep();
                ex->postStep();

                catch_up_step++;
              }

              if (!caught_up)
                throw MultiAppSolveFailure(name() + " Failed to catch up!\n");
            }
            else
              throw MultiAppSolveFailure(name() + " failed to converge");
          }
        }
      }

      // Re-enable all output (it may of been disabled by sub-cycling)
      problem.allowOutput(true);
    }

    _first = false;

    _console << "Successfully Solved MultiApp " << name() << "." << std::endl;
  }
  catch (MultiAppSolveFailure & e)
  {
    mooseWarning(e.what());
    _console << "Failed to Solve MultiApp " << name() << ", attempting to recover." << std::endl;
    return_value = false;
  }

  _transferred_vars.clear();

  return return_value;
}

void
TransientMultiApp::incrementTStep()
{
  if (!_sub_cycling)
  {
    for (unsigned int i = 0; i < _my_num_apps; i++)
    {
      Transient * ex = _transient_executioners[i];
      ex->incrementStepOrReject();
    }
  }
}

void
TransientMultiApp::finishStep()
{
  if (!_sub_cycling)
  {
    for (unsigned int i = 0; i < _my_num_apps; i++)
    {
      Transient * ex = _transient_executioners[i];
      ex->endStep();
      ex->postStep();
    }
  }
}

bool
TransientMultiApp::needsRestoration()
{
  return _sub_cycling || _catch_up || _auto_advance || _tolerate_failure || _detect_steady_state;
}

Real
TransientMultiApp::computeDT()
{
  if (_sub_cycling) // Bow out of the timestep selection dance
    return std::numeric_limits<Real>::max();

  Real smallest_dt = std::numeric_limits<Real>::max();

  if (_has_an_app)
  {
    Moose::ScopedCommSwapper swapper(_my_comm);

    for (unsigned int i = 0; i < _my_num_apps; i++)
    {
      Transient * ex = _transient_executioners[i];
      ex->computeDT();
      Real dt = ex->getDT();

      smallest_dt = std::min(dt, smallest_dt);
    }
  }

  if (_tolerate_failure) // Bow out of the timestep selection dance, we do this down here because we
                         // need to call computeConstrainedDT at least once for these
                         // executioners...
    return std::numeric_limits<Real>::max();

  _communicator.min(smallest_dt);
  return smallest_dt;
}

void TransientMultiApp::resetApp(
    unsigned int global_app,
    Real /*time*/) // FIXME: Note that we are passing in time but also grabbing it below
{
  if (hasLocalApp(global_app))
  {
    unsigned int local_app = globalAppToLocal(global_app);

    // Grab the current time the App is at so we can start the new one at the same place
    Real time =
        _transient_executioners[local_app]->getTime() + _apps[local_app]->getGlobalTimeOffset();

    // Reset the Multiapp
    MultiApp::resetApp(global_app, time);

    Moose::ScopedCommSwapper swapper(_my_comm);

    // Setup the app, disable the output so that the initial condition does not output
    // When an app is reset the initial condition was effectively already output before reset
    FEProblemBase & problem = appProblemBase(local_app);
    problem.allowOutput(false);
    setupApp(local_app, time);
    problem.allowOutput(true);
  }
}

void TransientMultiApp::setupApp(unsigned int i, Real /*time*/) // FIXME: Should we be passing time?
{
  auto & app = _apps[i];
  Transient * ex = dynamic_cast<Transient *>(app->getExecutioner());
  if (!ex)
    mooseError("MultiApp ", name(), " is not using a Transient Executioner!");

  // Get the FEProblemBase for the current MultiApp
  FEProblemBase & problem = appProblemBase(_first_local_app + i);

  // Update the file numbers for the outputs from the parent application
  app->getOutputWarehouse().setFileNumbers(_app.getOutputFileNumbers());

  // Call initialization method of Executioner (Note, this preforms the output of the initial time
  // step, if desired)
  ex->init();

  if (_interpolate_transfers)
  {
    AuxiliarySystem & aux_system = problem.getAuxiliarySystem();
    System & libmesh_aux_system = aux_system.system();

    // We'll store a copy of the auxiliary system's solution at the old time in here
    libmesh_aux_system.add_vector("transfer_old", false);

    // This will be where we'll transfer the value to for the "target" time
    libmesh_aux_system.add_vector("transfer", false);
  }

  ex->preExecute();
  if (!_app.isRecovering())
    problem.advanceState();
  _transient_executioners[i] = ex;
}
