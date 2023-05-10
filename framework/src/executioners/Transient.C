//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Transient.h"

// MOOSE includes
#include "Factory.h"
#include "SubProblem.h"
#include "TimeStepper.h"
#include "MooseApp.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "Control.h"
#include "TimePeriod.h"
#include "MooseMesh.h"
#include "TimeIntegrator.h"
#include "Console.h"
#include "AuxiliarySystem.h"

#include "libmesh/implicit_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/numeric_vector.h"

// C++ Includes
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

registerMooseObject("MooseApp", Transient);

InputParameters
Transient::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addClassDescription("Executioner for time varying simulations.");

  params += FEProblemSolve::validParams();

  std::vector<Real> sync_times(1);
  sync_times[0] = -std::numeric_limits<Real>::max();

  /**
   * For backwards compatibility we'll allow users to set the TimeIntegration scheme inside of the
   * executioner block
   * as long as the TimeIntegrator does not have any additional parameters.
   */
  MooseEnum schemes("implicit-euler explicit-euler crank-nicolson bdf2 explicit-midpoint dirk "
                    "explicit-tvd-rk-2 newmark-beta",
                    "implicit-euler");

  params.addParam<Real>("start_time", 0.0, "The start time of the simulation");
  params.addParam<Real>("end_time", 1.0e30, "The end time of the simulation");
  params.addParam<Real>("dt", 1., "The timestep size between solves");
  params.addParam<Real>("dtmin", 1.0e-12, "The minimum timestep size in an adaptive run");
  params.addParam<Real>("dtmax", 1.0e30, "The maximum timestep size in an adaptive run");
  params.addParam<bool>(
      "reset_dt", false, "Use when restarting a calculation to force a change in dt.");
  params.addParam<unsigned int>("num_steps",
                                std::numeric_limits<unsigned int>::max(),
                                "The number of timesteps in a transient run");
  params.addParam<int>("n_startup_steps", 0, "The number of timesteps during startup");

  params.addDeprecatedParam<bool>("trans_ss_check",
                                  false,
                                  "Whether or not to check for steady state conditions",
                                  "Use steady_state_detection instead");
  params.addDeprecatedParam<Real>("ss_check_tol",
                                  1.0e-08,
                                  "Whenever the relative residual changes by less "
                                  "than this the solution will be considered to be "
                                  "at steady state.",
                                  "Use steady_state_tolerance instead");
  params.addDeprecatedParam<Real>(
      "ss_tmin",
      0.0,
      "Minimum amount of time to run before checking for steady state conditions.",
      "Use steady_state_start_time instead");

  params.addParam<bool>(
      "steady_state_detection", false, "Whether or not to check for steady state conditions");
  params.addParam<Real>("steady_state_tolerance",
                        1.0e-08,
                        "Whenever the relative residual changes by less "
                        "than this the solution will be considered to be "
                        "at steady state.");
  params.addParam<Real>(
      "steady_state_start_time",
      0.0,
      "Minimum amount of time to run before checking for steady state conditions.");
  params.addParam<bool>(
      "normalize_solution_diff_norm_by_dt",
      true,
      "Whether to divide the solution difference norm by dt. If taking 'small' "
      "time steps you probably want this to be true. If taking very 'large' timesteps in an "
      "attempt to *reach* a steady-state, you probably want this parameter to be false.");

  params.addParam<std::vector<std::string>>("time_periods", "The names of periods");
  params.addParam<std::vector<Real>>("time_period_starts", "The start times of time periods");
  params.addParam<std::vector<Real>>("time_period_ends", "The end times of time periods");
  params.addParam<bool>(
      "abort_on_solve_fail", false, "abort if solve not converged rather than cut timestep");
  params.addParam<bool>(
      "error_on_dtmin",
      true,
      "Throw error when timestep is less than dtmin instead of just aborting solve.");
  params.addParam<MooseEnum>("scheme", schemes, "Time integration scheme used.");
  params.addParam<Real>("timestep_tolerance",
                        1.0e-12,
                        "the tolerance setting for final timestep size and sync times");

  params.addParam<bool>("use_multiapp_dt",
                        false,
                        "If true then the dt for the simulation will be "
                        "chosen by the MultiApps.  If false (the "
                        "default) then the minimum over the master dt "
                        "and the MultiApps is used");

  params.addParam<bool>("check_aux",
                        false,
                        "Whether to check the auxiliary system for convergence to steady-state. If "
                        "false, then the nonlinear system is used.");

  params.addParamNamesToGroup(
      "steady_state_detection steady_state_tolerance steady_state_start_time check_aux",
      "Steady State Detection");

  params.addParamNamesToGroup("start_time dtmin dtmax n_startup_steps trans_ss_check ss_check_tol "
                              "ss_tmin abort_on_solve_fail timestep_tolerance use_multiapp_dt",
                              "Advanced");

  params.addParamNamesToGroup("time_periods time_period_starts time_period_ends", "Time Periods");

  return params;
}

Transient::Transient(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _feproblem_solve(*this),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _aux(_fe_problem.getAuxiliarySystem()),
    _check_aux(getParam<bool>("check_aux")),
    _time_scheme(getParam<MooseEnum>("scheme").getEnum<Moose::TimeIntegratorType>()),
    _time_stepper(nullptr),
    _t_step(_problem.timeStep()),
    _time(_problem.time()),
    _time_old(_problem.timeOld()),
    _dt(_problem.dt()),
    _dt_old(_problem.dtOld()),
    _unconstrained_dt(declareRecoverableData<Real>("unconstrained_dt", -1)),
    _at_sync_point(declareRecoverableData<bool>("at_sync_point", false)),
    _last_solve_converged(declareRecoverableData<bool>("last_solve_converged", true)),
    _xfem_repeat_step(false),
    _end_time(getParam<Real>("end_time")),
    _dtmin(getParam<Real>("dtmin")),
    _dtmax(getParam<Real>("dtmax")),
    _num_steps(getParam<unsigned int>("num_steps")),
    _n_startup_steps(getParam<int>("n_startup_steps")),
    _steady_state_detection(getParam<bool>("steady_state_detection")),
    _steady_state_tolerance(getParam<Real>("steady_state_tolerance")),
    _steady_state_start_time(getParam<Real>("steady_state_start_time")),
    _sync_times(_app.getOutputWarehouse().getSyncTimes()),
    _abort(getParam<bool>("abort_on_solve_fail")),
    _error_on_dtmin(getParam<bool>("error_on_dtmin")),
    _time_interval(declareRecoverableData<bool>("time_interval", false)),
    _start_time(getParam<Real>("start_time")),
    _timestep_tolerance(getParam<Real>("timestep_tolerance")),
    _target_time(declareRecoverableData<Real>("target_time", -std::numeric_limits<Real>::max())),
    _use_multiapp_dt(getParam<bool>("use_multiapp_dt")),
    _solution_change_norm(declareRecoverableData<Real>("solution_change_norm", 0.0)),
    _sln_diff(_check_aux ? _aux.addVector("sln_diff", false, PARALLEL)
                         : _nl.addVector("sln_diff", false, PARALLEL)),
    _normalize_solution_diff_norm_by_dt(getParam<bool>("normalize_solution_diff_norm_by_dt"))
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);

  // Handle deprecated parameters
  if (!parameters.isParamSetByAddParam("trans_ss_check"))
    _steady_state_detection = getParam<bool>("trans_ss_check");

  if (!parameters.isParamSetByAddParam("ss_check_tol"))
    _steady_state_tolerance = getParam<Real>("ss_check_tol");

  if (!parameters.isParamSetByAddParam("ss_tmin"))
    _steady_state_start_time = getParam<Real>("ss_tmin");

  _t_step = 0;
  _dt = 0;
  _next_interval_output_time = 0.0;

  // Either a start_time has been forced on us, or we want to tell the App about what our start time
  // is (in case anyone else is interested.
  if (_app.hasStartTime())
    _start_time = _app.getStartTime();
  else if (parameters.isParamSetByUser("start_time"))
    _app.setStartTime(_start_time);

  _time = _time_old = _start_time;
  _problem.transient(true);

  setupTimeIntegrator();

  if (_app.halfTransient()) // Cut timesteps and end_time in half...
  {
    _end_time /= 2.0;
    _num_steps /= 2.0;

    if (_num_steps == 0) // Always do one step in the first half
      _num_steps = 1;
  }
}

void
Transient::init()
{
  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _problem.initialSetup();

  mooseAssert(getTimeStepper(), "No time stepper was set");

  /**
   * If this is a restart run, the user may want to override the start time, which we already set in
   * the constructor. "_time" however will have been "restored" from the restart file. We need to
   * honor the original request of the developer now that the restore has been completed. This must
   * occur before we init the time stepper (since that prints out the start time). The multiapp case
   * is also bit complicated. If we didn't set a start time, the app won't have it yet, so we just
   * restart the old time from the current time.
   */
  if (_app.isRestarting())
  {
    if (_app.hasStartTime())
      _time = _time_old = _app.getStartTime();
    else
      _time_old = _time;
  }

  _time_stepper->init();

  if (_app.isRecovering()) // Recover case
  {
    if (_t_step == 0)
      mooseError("Internal error in Transient executioner: _t_step is equal to 0 while recovering "
                 "in init().");

    _dt_old = _dt;
  }
}

void
Transient::preExecute()
{
  _time_stepper->preExecute();

  if (!_app.isRecovering())
  {
    _t_step = 0;
    _dt = 0;
    _next_interval_output_time = 0.0;
    if (!_app.isRestarting())
      _time = _time_old = _start_time;

    _problem.outputStep(EXEC_INITIAL);

    computeDT();
    _dt = getDT();
    if (_dt == 0)
      mooseError("Time stepper computed zero time step size on initial which is not allowed.\n"
                 "1. If you are using an existing time stepper, double check the values in your "
                 "input file or report an error.\n"
                 "2. If you are developing a new time stepper, make sure that initial time step "
                 "size in your code is computed correctly.");
    _nl.getTimeIntegrator()->init();
  }
}

void
Transient::preStep()
{
  _time_stepper->preStep();
}

void
Transient::postStep()
{
  _time_stepper->postStep();
}

void
Transient::execute()
{
  preExecute();

  // Start time loop...
  while (keepGoing())
  {
    incrementStepOrReject();
    preStep();
    computeDT();
    takeStep();
    endStep();
    postStep();
  }

  if (lastSolveConverged())
  {
    _t_step++;

    /*
     * Call the multi-app executioners endStep and
     * postStep methods when doing Picard or when not automatically advancing sub-applications for
     * some other reason. We do not perform these calls for loose-coupling/auto-advancement
     * problems because Transient::endStep and Transient::postStep get called from
     * TransientMultiApp::solveStep in that case.
     */
    if (!_fixed_point_solve->autoAdvance())
    {
      _problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN,
                                  /*recurse_through_multiapp_levels=*/true);
      _problem.finishMultiAppStep(EXEC_TIMESTEP_BEGIN, /*recurse_through_multiapp_levels=*/true);
      _problem.finishMultiAppStep(EXEC_TIMESTEP_END, /*recurse_through_multiapp_levels=*/true);
      _problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_END,
                                  /*recurse_through_multiapp_levels=*/true);
    }
  }

  if (!_app.halfTransient())
  {
    TIME_SECTION("final", 1, "Executing Final Objects");
    _problem.execMultiApps(EXEC_FINAL);
    _problem.finalizeMultiApps();
    _problem.execute(EXEC_FINAL);
    _problem.outputStep(EXEC_FINAL);
  }

  // This method is to finalize anything else we want to do on the problem side.
  _problem.postExecute();

  // This method can be overridden for user defined activities in the Executioner.
  postExecute();
}

void
Transient::computeDT()
{
  _time_stepper->computeStep(); // This is actually when DT gets computed
}

void
Transient::incrementStepOrReject()
{
  if (lastSolveConverged())
  {
    if (!_xfem_repeat_step)
    {
#ifdef LIBMESH_ENABLE_AMR
      if (_t_step != 0)
        _problem.adaptMesh();
#endif

      _time_old = _time;
      _t_step++;

      _problem.advanceState();

      if (_t_step == 1)
        return;

      /*
       * Call the multi-app executioners endStep and
       * postStep methods when doing Picard or when not automatically advancing sub-applications for
       * some other reason. We do not perform these calls for loose-coupling/auto-advancement
       * problems because Transient::endStep and Transient::postStep get called from
       * TransientMultiApp::solveStep in that case.
       */
      if (!_fixed_point_solve->autoAdvance())
      {
        _problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
        _problem.finishMultiAppStep(EXEC_TIMESTEP_BEGIN);
        _problem.finishMultiAppStep(EXEC_TIMESTEP_END);
        _problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_END);
      }

      /*
       * Ensure that we increment the sub-application time steps so that
       * when dt selection is made in the master application, we are using
       * the correct time step information
       */
      _problem.incrementMultiAppTStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
      _problem.incrementMultiAppTStep(EXEC_TIMESTEP_BEGIN);
      _problem.incrementMultiAppTStep(EXEC_TIMESTEP_END);
      _problem.incrementMultiAppTStep(EXEC_MULTIAPP_FIXED_POINT_END);
    }
  }
  else
  {
    _problem.restoreMultiApps(EXEC_MULTIAPP_FIXED_POINT_BEGIN, true);
    _problem.restoreMultiApps(EXEC_TIMESTEP_BEGIN, true);
    _problem.restoreMultiApps(EXEC_TIMESTEP_END, true);
    _problem.restoreMultiApps(EXEC_MULTIAPP_FIXED_POINT_END, true);
    _time_stepper->rejectStep();
    _time = _time_old;
  }
}

void
Transient::takeStep(Real input_dt)
{
  _dt_old = _dt;

  if (input_dt == -1.0)
    _dt = computeConstrainedDT();
  else
    _dt = input_dt;

  _time_stepper->preSolve();

  // Increment time
  _time = _time_old + _dt;

  _problem.timestepSetup();

  _problem.onTimestepBegin();

  _time_stepper->step();
  _xfem_repeat_step = _fixed_point_solve->XFEMRepeatStep();

  _last_solve_converged = _time_stepper->converged();

  if (!lastSolveConverged())
  {
    _console << "Aborting as solve did not converge" << std::endl;
    return;
  }

  if (!(_problem.haveXFEM() && _fixed_point_solve->XFEMRepeatStep()))
  {
    if (lastSolveConverged())
      _time_stepper->acceptStep();
    else
      _time_stepper->rejectStep();
  }

  _time = _time_old;

  _time_stepper->postSolve();

  _solution_change_norm =
      relativeSolutionDifferenceNorm() / (_normalize_solution_diff_norm_by_dt ? _dt : Real(1));

  return;
}

void
Transient::endStep(Real input_time)
{
  if (input_time == -1.0)
    _time = _time_old + _dt;
  else
    _time = input_time;

  if (lastSolveConverged())
  {
    if (_xfem_repeat_step)
      _time = _time_old;
    else
    {
      _nl.getTimeIntegrator()->postStep();

      // Compute the Error Indicators and Markers
      _problem.computeIndicators();
      _problem.computeMarkers();

      // Perform the output of the current time step
      _problem.outputStep(EXEC_TIMESTEP_END);

      // output
      if (_time_interval && (_time + _timestep_tolerance >= _next_interval_output_time))
        _next_interval_output_time += _time_interval_output_interval;
    }
  }
}

Real
Transient::computeConstrainedDT()
{
  //  // If start up steps are needed
  //  if (_t_step == 1 && _n_startup_steps > 1)
  //    _dt = _input_dt/(double)(_n_startup_steps);
  //  else if (_t_step == 1+_n_startup_steps && _n_startup_steps > 1)
  //    _dt = _input_dt;

  Real dt_cur = _dt;
  std::ostringstream diag;

  // After startup steps, compute new dt
  if (_t_step > _n_startup_steps)
    dt_cur = getDT();

  else
  {
    diag << "Timestep < n_startup_steps, using old dt: " << std::setw(9) << std::setprecision(6)
         << std::setfill('0') << std::showpoint << std::left << _dt << " tstep: " << _t_step
         << " n_startup_steps: " << _n_startup_steps << std::endl;
  }
  _unconstrained_dt = dt_cur;

  if (_verbose)
    _console << diag.str();

  diag.str("");
  diag.clear();

  // Allow the time stepper to limit the time step
  _at_sync_point = _time_stepper->constrainStep(dt_cur);

  // Don't let time go beyond next time interval output if specified
  if ((_time_interval) && (_time + dt_cur + _timestep_tolerance >= _next_interval_output_time))
  {
    dt_cur = _next_interval_output_time - _time;
    _at_sync_point = true;

    diag << "Limiting dt for time interval output at time: " << std::setw(9) << std::setprecision(6)
         << std::setfill('0') << std::showpoint << std::left << _next_interval_output_time
         << " dt: " << std::setw(9) << std::setprecision(6) << std::setfill('0') << std::showpoint
         << std::left << dt_cur << std::endl;
  }

  // If a target time is set and the current dt would exceed it, limit dt to match the target
  if (_target_time > -std::numeric_limits<Real>::max() + _timestep_tolerance &&
      _time + dt_cur + _timestep_tolerance >= _target_time)
  {
    dt_cur = _target_time - _time;
    _at_sync_point = true;

    diag << "Limiting dt for target time: " << std::setw(9) << std::setprecision(6)
         << std::setfill('0') << std::showpoint << std::left << _next_interval_output_time
         << " dt: " << std::setw(9) << std::setprecision(6) << std::setfill('0') << std::showpoint
         << std::left << dt_cur << std::endl;
  }

  // Constrain by what the multi apps are doing
  constrainDTFromMultiApp(dt_cur, diag, EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  constrainDTFromMultiApp(dt_cur, diag, EXEC_TIMESTEP_BEGIN);
  constrainDTFromMultiApp(dt_cur, diag, EXEC_TIMESTEP_END);
  constrainDTFromMultiApp(dt_cur, diag, EXEC_MULTIAPP_FIXED_POINT_END);

  if (_verbose)
    _console << diag.str();

  return dt_cur;
}

void
Transient::constrainDTFromMultiApp(Real & dt_cur,
                                   std::ostringstream & diag,
                                   const ExecFlagType & execute_on) const
{
  Real multi_app_dt = _problem.computeMultiAppsDT(execute_on);
  if (_use_multiapp_dt || multi_app_dt < dt_cur)
  {
    dt_cur = multi_app_dt;
    _at_sync_point = false;
    diag << "Limiting dt for MultiApps on " << execute_on.name() << ": " << std::setw(9)
         << std::setprecision(6) << std::setfill('0') << std::showpoint << std::left << dt_cur
         << std::endl;
  }
}

Real
Transient::getDT()
{
  return _time_stepper->getCurrentDT();
}

bool
Transient::keepGoing()
{
  bool keep_going = !_problem.isSolveTerminationRequested();

  // Check for stop condition based upon steady-state check flag:
  if (lastSolveConverged())
  {
    if (!_xfem_repeat_step)
    {
      if (_steady_state_detection == true && _time > _steady_state_start_time)
      {
        // Check solution difference relative norm against steady-state tolerance
        if (_solution_change_norm < _steady_state_tolerance)
        {
          _console << "Steady-State Solution Achieved at time: " << _time << std::endl;
          // Output last solve if not output previously by forcing it
          keep_going = false;
        }
        else // Keep going
        {
          // Print steady-state relative error norm
          _console << "Steady-State Relative Differential Norm: " << _solution_change_norm
                   << std::endl;
        }
      }

      // Check for stop condition based upon number of simulation steps and/or solution end time:
      if (static_cast<unsigned int>(_t_step) >= _num_steps)
        keep_going = false;

      if ((_time >= _end_time) || (fabs(_time - _end_time) <= _timestep_tolerance))
        keep_going = false;
    }
  }
  else if (_abort)
  {
    _console << "Aborting as solve did not converge and input selected to abort" << std::endl;
    keep_going = false;
  }
  else if (!_error_on_dtmin && _dt <= _dtmin)
  {
    _console << "Aborting as timestep already at or below dtmin" << std::endl;
    keep_going = false;
  }

  return keep_going;
}

void
Transient::estimateTimeError()
{
}

bool
Transient::lastSolveConverged() const
{
  return _last_solve_converged;
}

void
Transient::postExecute()
{
  _time_stepper->postExecute();
}

void
Transient::setTargetTime(Real target_time)
{
  _target_time = target_time;
}

Real
Transient::getSolutionChangeNorm()
{
  return _solution_change_norm;
}

void
Transient::setupTimeIntegrator()
{
  if (_pars.isParamSetByUser("scheme") && _problem.hasTimeIntegrator())
    mooseError("You cannot specify time_scheme in the Executioner and independently add a "
               "TimeIntegrator to the system at the same time");

  if (!_problem.hasTimeIntegrator())
  {
    // backwards compatibility
    std::string ti_str;
    using namespace Moose;

    switch (_time_scheme)
    {
      case TI_IMPLICIT_EULER:
        ti_str = "ImplicitEuler";
        break;
      case TI_EXPLICIT_EULER:
        ti_str = "ExplicitEuler";
        break;
      case TI_CRANK_NICOLSON:
        ti_str = "CrankNicolson";
        break;
      case TI_BDF2:
        ti_str = "BDF2";
        break;
      case TI_EXPLICIT_MIDPOINT:
        ti_str = "ExplicitMidpoint";
        break;
      case TI_LSTABLE_DIRK2:
        ti_str = "LStableDirk2";
        break;
      case TI_EXPLICIT_TVD_RK_2:
        ti_str = "ExplicitTVDRK2";
        break;
      case TI_NEWMARK_BETA:
        ti_str = "NewmarkBeta";
        break;
      default:
        mooseError("Unknown scheme: ", _time_scheme);
        break;
    }

    InputParameters params = _app.getFactory().getValidParams(ti_str);
    _problem.addTimeIntegrator(ti_str, ti_str, params);
  }
}

std::string
Transient::getTimeStepperName() const
{
  if (_time_stepper)
  {
    TimeStepper & ts = *_time_stepper;
    return demangle(typeid(ts).name());
  }
  else
    return std::string();
}

std::string
Transient::getTimeIntegratorName() const
{
  const auto * ti = _nl.getTimeIntegrator();
  if (ti)
    return ti->type();
  else
    mooseError("Time integrator has not been built yet so we can't retrieve its name");
}

Real
Transient::relativeSolutionDifferenceNorm()
{
  const NumericVector<Number> & current_solution =
      _check_aux ? _aux.solution() : *_nl.currentSolution();
  const NumericVector<Number> & old_solution = _check_aux ? _aux.solutionOld() : _nl.solutionOld();

  _sln_diff = current_solution;
  _sln_diff -= old_solution;

  return (_sln_diff.l2_norm() / current_solution.l2_norm());
}

void
Transient::setTimeStepper(TimeStepper & ts)
{
  mooseAssert(!_time_stepper, "Already set");
  _time_stepper = &ts;
}
