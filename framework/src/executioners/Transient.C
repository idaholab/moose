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

#include "Transient.h"

//Moose includes
#include "Kernel.h"
#include "Factory.h"
#include "SubProblem.h"
#include "TimePeriod.h"
#include "TimeStepper.h"
#include "MooseApp.h"
#include "Conversion.h"
//libMesh includes
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

template<>
InputParameters validParams<Transient>()
{
  InputParameters params = validParams<Executioner>();
  std::vector<Real> sync_times(1);
  sync_times[0] = -std::numeric_limits<Real>::max();
  MooseEnum schemes("implicit-euler, explicit-euler, crank-nicolson, bdf2", "implicit-euler");

  params.addParam<Real>("start_time",      0.0,    "The start time of the simulation");
  params.addParam<Real>("end_time",        1.0e30, "The end time of the simulation");
  params.addParam<Real>("dt",              1.,     "The timestep size between solves");
  params.addParam<Real>("dtmin",           2.0e-14,    "The minimum timestep size in an adaptive run");
  params.addParam<Real>("dtmax",           1.0e30, "The maximum timestep size in an adaptive run");
  params.addParam<Real>("num_steps",       std::numeric_limits<Real>::max(),     "The number of timesteps in a transient run");
  params.addParam<int> ("n_startup_steps", 0,      "The number of timesteps during startup");
  params.addParam<bool>("trans_ss_check",  false,  "Whether or not to check for steady state conditions");
  params.addParam<Real>("ss_check_tol",    1.0e-08,"Whenever the relative residual changes by less than this the solution will be considered to be at steady state.");
  params.addParam<Real>("ss_tmin",         0.0,    "Minimum number of timesteps to take before checking for steady state conditions.");
  params.addParam<std::vector<Real> >("sync_times", sync_times, "A list of times that will be solved for provided they are within the simulation time");
  params.addParam<Real>("predictor_scale", "The scale factor for the predictor (can range from 0 to 1)");

  params.addParam<std::vector<std::string> >("time_periods", "The names of periods");
  params.addParam<std::vector<Real> >("time_period_starts", "The start times of time periods");
  params.addParam<std::vector<Real> >("time_period_ends", "The end times of time periods");
  params.addParam<bool>("abort_on_solve_fail", false, "abort if solve not converged rather than cut timestep");
  params.addParam<MooseEnum>("scheme",          schemes,  "Time integration scheme used.");
  params.addParam<Real>("timestep_tolerance", 2.0e-14, "the tolerance setting for final timestep size and sync times");

  params.addParam<bool>("use_multiapp_dt", false, "If true then the dt for the simulation will be chosen by the MultiApps.  If false (the default) then the minimum over the master dt and the MultiApps is used");

  params.addParamNamesToGroup("start_time dtmin dtmax n_startup_steps trans_ss_check ss_check_tol ss_tmin sync_times time_t time_dt growth_factor predictor_scale use_AB2 use_littlef abort_on_solve_fail output_to_file file_name estimate_time_error timestep_tolerance use_multiapp_dt", "Advanced");

  params.addParamNamesToGroup("time_periods time_period_starts time_period_ends", "Time Periods");

  return params;
}

Transient::Transient(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(*getParam<FEProblem *>("_fe_problem")),
    _time_scheme(getParam<MooseEnum>("scheme")),
    _time_stepper(NULL),
    _t_step(_problem.timeStep()),
    _time(_problem.time()),
    _time_old(_problem.timeOld()),
    _dt(_problem.dt()),
    _dt_old(_problem.dtOld()),
    _unconstrained_dt(0),
    _unconstrained_dt_old(0),
    _prev_dt(-1),
    _reset_dt(false),
    _end_time(getParam<Real>("end_time")),
    _dtmin(getParam<Real>("dtmin")),
    _dtmax(getParam<Real>("dtmax")),
    _num_steps(getParam<Real>("num_steps")),
    _n_startup_steps(getParam<int>("n_startup_steps")),
    _trans_ss_check(getParam<bool>("trans_ss_check")),
    _ss_check_tol(getParam<Real>("ss_check_tol")),
    _ss_tmin(getParam<Real>("ss_tmin")),
    _old_time_solution_norm(0.0),
    _sync_times(getParam<std::vector<Real> >("sync_times").begin(),getParam<std::vector<Real> >("sync_times").end()),
    _remaining_sync_time(true),
    _abort(getParam<bool>("abort_on_solve_fail")),
    _time_interval(false),
    _start_time(getParam<Real>("start_time")),
    _timestep_tolerance(getParam<Real>("timestep_tolerance")),
    _target_time(-1),
    _use_multiapp_dt(getParam<bool>("use_multiapp_dt")),
    _allow_output(true)
{
  _t_step = 0;
  _dt = 0;
  _time = _time_old = _start_time;
  _problem.transient(true);

  if (parameters.isParamValid("predictor_scale"))
  {
    Real predscale = getParam<Real>("predictor_scale");
    if (predscale >= 0.0 and predscale <= 1.0)
    {
      InputParameters params = _app.getFactory().getValidParams("Predictor");
      params.set<Real>("scale") = predscale;
      _problem.addPredictor("Predictor", "predictor", params);
    }
    else
    {
      mooseError("Input value for predictor_scale = "<< predscale << ", outside of permissible range (0 to 1)");
    }
  }

  if (!_restart_file_base.empty())
    _problem.setRestartFile(_restart_file_base);

  setupTimeIntegrator();
}

Transient::~Transient()
{
  delete _time_stepper;
  // This problem was built by the Factory and needs to be released by this destructor
  delete &_problem;
}

void
Transient::init()
{
  if (_time_stepper == NULL)
  {
    mooseWarning("Time stepper not set, defaulting in constant time stepping...");
    InputParameters pars = _app.getFactory().getValidParams("ConstantDT");
    pars.set<FEProblem *>("_fe_problem") = &_problem;
    pars.set<Transient *>("_executioner") = this;
    pars.set<Real>("dt") = getParam<Real>("dt");
    _time_stepper = static_cast<TimeStepper *>(_app.getFactory().create("ConstantDT", "TimeStepper", pars));
  }

  _problem.initialSetup();
  _time_stepper->init();

  Moose::setup_perf_log.push("Output Initial Condition","Setup");
  if (_output_initial)
  {
    _problem.output();
    _problem.outputPostprocessors();
  }
  Moose::setup_perf_log.pop("Output Initial Condition","Setup");
}

void
Transient::execute()
{
  preExecute();
  _time_stepper->preExecute();

  // NOTE: if you remove this line, you will see a subset of tests failing. Those tests might have a wrong answer and might need to be regolded.
  // The reason is that we actually move the solution back in time before we actually start solving (which I think is wrong).  So this call here
  // is to maintain backward compatibility and so that MOOSE is giving the same answer.  However, we might remove this call and regold the test
  // in the future eventually.
  _problem.copyOldSolutions();

  // Start time loop...
  while (keepGoing())
  {
    takeStep();
    endStep();
  }

  postExecute();
  _time_stepper->postExecute();
}

void
Transient::takeStep(Real input_dt)
{
  _problem.out().setOutput(false);

  _dt_old = _dt;
  _unconstrained_dt_old = _unconstrained_dt;

  if (input_dt == -1.0)
    _dt = computeConstrainedDT();
  else
    _dt = input_dt;

  _problem.onTimestepBegin();
  if (lastSolveConverged())
    _problem.updateMaterials();             // Update backward material data structures

  // Increment time
  _time = _time_old + _dt;

  _problem.execTransfers(EXEC_TIMESTEP_BEGIN);
  _problem.execMultiApps(EXEC_TIMESTEP_BEGIN);

  std::cout<<"DT: "<<_dt<<std::endl;

  std::cout << " Solving time step ";
  {
    std::ostringstream out;

    out << std::setw(2)
        << _t_step
        << ", time="
        << std::setw(9)
        << std::setprecision(6)
        << std::setfill('0')
        << std::showpoint
        << std::left
        << _time
        <<  "...";
    std::cout << out.str() << std::endl;
  }

  preSolve();
  _time_stepper->preSolve();

  _problem.timestepSetup();

  // Compute Pre-Aux User Objects (Timestep begin)
  _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN, UserObjectWarehouse::PRE_AUX);

  // Compute TimestepBegin AuxKernels
  _problem.computeAuxiliaryKernels(EXEC_TIMESTEP_BEGIN);

  // Compute Post-Aux User Objects (Timestep begin)
  _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN, UserObjectWarehouse::POST_AUX);

  _time_stepper->step();

  // We know whether or not the nonlinear solver thinks it converged, but we need to see if the executioner concurs
  if (lastSolveConverged())
  {
    std::cout<<"Solve Converged!"<<std::endl;

    computeSolutionChangeNorm();

    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);
#if 0
    // User definable callback
    if(_estimate_error)
    {
      estimateTimeError();
    }
#endif

    _problem.onTimestepEnd();

    _problem.computeAuxiliaryKernels(EXEC_TIMESTEP);
    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::POST_AUX);
    _problem.execTransfers(EXEC_TIMESTEP);
    _problem.execMultiApps(EXEC_TIMESTEP);
  }
  else
    std::cout<<"Solve Did NOT Converge!"<<std::endl;

  postSolve();
  _time_stepper->postSolve();
}

void
Transient::endStep()
{
  if (lastSolveConverged())
  {
    // Compute the Error Indicators and Markers
    _problem.computeIndicatorsAndMarkers();

    //output
    if(_time_interval)
    {
      if(std::abs(_time-(_prev_sync_time))<=_dtmin || (_t_step % _problem.out().interval() == 0 && _problem.out().interval() > 1))
      {
        if(_allow_output)
        {
          _problem.output(true);
          _problem.outputPostprocessors(true);
        }
      }
    }
    else
    {
      // if _reset_dt is true, force the output no matter what
      if(_allow_output)
      {
        _problem.output(_reset_dt);
        _problem.outputPostprocessors(_reset_dt);
      }
    }
#ifdef LIBMESH_ENABLE_AMR
    if (_problem.adaptivity().isOn())
    {
      _problem.adaptMesh();
    }
#endif

    _time_old = _time;
    _t_step++;

    _problem.copyOldSolutions();
  }
  else
  {
    _time_stepper->rejectStep();
  }
}

Real
Transient::computeConstrainedDT()
{
  // If this is the first step
  if (_t_step == 0)
  {
    _t_step = 1;
    _dt = computeDT();
  }

//  // If start up steps are needed
//  if(_t_step == 1 && _n_startup_steps > 1)
//    _dt = _input_dt/(double)(_n_startup_steps);
//  else if (_t_step == 1+_n_startup_steps && _n_startup_steps > 1)
//    _dt = _input_dt;

  Real dt_cur = _dt;

  //After startup steps, compute new dt
  if (_t_step > _n_startup_steps)
  {
    _unconstrained_dt = computeDT();
    dt_cur = _unconstrained_dt;
  }

  // Don't let the time step size exceed maximum time step size
  if (dt_cur > _dtmax)
    dt_cur = _dtmax;

  // Don't allow time step size to be smaller than minimum time step size
  if (dt_cur < _dtmin)
    dt_cur = _dtmin;

  // Don't let time go beyond simulation end time
  if (_time + dt_cur > _end_time)
    dt_cur = _end_time - _time;

  // Adjust to a sync time if supplied and skipped over
  if (_remaining_sync_time && _time + dt_cur + _timestep_tolerance >= (*_sync_times.begin()))
  {
    if (fabs(*_sync_times.begin() - _time) >= _timestep_tolerance)
    {
      dt_cur = *_sync_times.begin() - _time;
    }
    _prev_sync_time = *_sync_times.begin();
    _sync_times.erase(_sync_times.begin());
    if(_time_interval)
    {
      Real d = (_time + dt_cur - _start_time) / _time_interval_output_interval;
      if (d-std::floor(d) <= _timestep_tolerance || std::ceil(d)-d <= _timestep_tolerance)
      {
        _sync_times.insert((_time + dt_cur + _time_interval_output_interval));
      }
    }
    if (_sync_times.begin() == _sync_times.end())
      _remaining_sync_time = false;

    _prev_dt = dt_cur;
    _reset_dt = true;
  }
  else
  {
    if (_reset_dt)
    {
      dt_cur = computeDT();
      _reset_dt = false;
    }
  }

  // Constrain by what the multi apps are doing
  Real multi_app_dt = _problem.computeMultiAppsDT(EXEC_TIMESTEP_BEGIN);
  if(_use_multiapp_dt || multi_app_dt < dt_cur)
    dt_cur = multi_app_dt;
  multi_app_dt = _problem.computeMultiAppsDT(EXEC_TIMESTEP);
  if(multi_app_dt < dt_cur)
    dt_cur = multi_app_dt;

  // Adjust to a target time if set
  if (_target_time > 0 && _time + dt_cur + _timestep_tolerance >= _target_time)
  {
    dt_cur = _target_time - _time;

    _prev_dt = _dt;
    _reset_dt = true;
  }

  return dt_cur;
}

Real
Transient::computeDT()
{
  return _time_stepper->computeDT();
}

bool
Transient::keepGoing()
{
  bool keep_going = true;
  // Check for stop condition based upon steady-state check flag:
  if(lastSolveConverged() && _trans_ss_check == true && _time > _ss_tmin)
  {
    // Compute new time solution l2_norm
    Real new_time_solution_norm = _problem.getNonlinearSystem().currentSolution()->l2_norm();

    // Compute l2_norm relative error
    Real ss_relerr_norm = fabs(new_time_solution_norm - _old_time_solution_norm)/new_time_solution_norm;

    // Check current solution relative error norm against steady-state tolerance
    if(ss_relerr_norm < _ss_check_tol)
    {
      std::cout<<"Steady-State Solution Achieved at time: "<<_time<<std::endl;
      //Output last solve if not output previously by forcing it
      keep_going = false;
    }
    else // Keep going
    {
      // Update solution norm for next time step
      _old_time_solution_norm = new_time_solution_norm;
      // Print steady-state relative error norm
      std::cout<<"Steady-State Relative Differential Norm: "<<ss_relerr_norm<<std::endl;
    }
  }

  // Check for stop condition based upon number of simulation steps and/or solution end time:
  if(_t_step>_num_steps)
    keep_going = false;

  if((_time>_end_time) || (fabs(_time-_end_time)<_timestep_tolerance))
    keep_going = false;

  if(!keep_going && !_problem.out().wasOutput())
  {
    _problem.output(true);
    if(_allow_output)
      _problem.outputPostprocessors(true);
  }

  if(!lastSolveConverged() && _abort)
  {
    std::cout<<"Aborting as solve did not converge and input selected to abort"<<std::endl;
    keep_going = false;
  }
  return keep_going;
}

void
Transient::estimateTimeError()
{
}

bool
Transient::lastSolveConverged()
{
  return _time_stepper->converged();
}

void
Transient::preExecute()
{
  if(_problem.out().useTimeInterval())
  {
    _time_interval = true;
    _time_interval_output_interval = _problem.out().timeinterval();
    _sync_times.insert((_time + _time_interval_output_interval));
  }
  // process time periods
  const std::vector<TimePeriod *> time_periods = _problem.getTimePeriods();
  for (unsigned int i = 0; i < time_periods.size(); ++i)
    _sync_times.insert(time_periods[i]->start());

  // Advance to the first sync time if one is provided in sim time range
  while (_remaining_sync_time && *_sync_times.begin() <= _time)
  {
    _sync_times.erase(_sync_times.begin());
    if (_sync_times.begin() == _sync_times.end())
      _remaining_sync_time = false;
  }
  if(_remaining_sync_time)
  {
    _prev_sync_time = *_sync_times.begin();
  }
}

Problem &
Transient::problem()
{
  return _problem;
}

void
Transient::setTargetTime(Real target_time)
{
  _target_time = target_time;
}

void
Transient::forceOutput()
{
  _problem.output(true);
  _problem.outputPostprocessors(true);
}

void
Transient::computeSolutionChangeNorm()
{
  NumericVector<Number> & current_solution  = (*_problem.getNonlinearSystem().sys().current_local_solution);
  NumericVector<Number> & old_solution = (*_problem.getNonlinearSystem().sys().old_local_solution);

  NumericVector<Number> & difference = *NumericVector<Number>::build().release();
  difference.init(current_solution, true);

  difference = current_solution;

  difference -= old_solution;

  Real abs_change = difference.l2_norm();

  delete &difference;

  _solution_change_norm = (abs_change / current_solution.l2_norm()) / _dt;
}

Real
Transient::getSolutionChangeNorm()
{
  return _solution_change_norm;
}

void
Transient::setupTimeIntegrator()
{
  // backwards compatibility
  std::string ti_str;

  switch (_time_scheme)
  {
  case 0: ti_str = "ImplicitEuler"; break;
  case 1: ti_str = "ExplicitEuler"; break;
  case 2: ti_str = "CrankNicolson"; break;
  case 3: ti_str = "BDF2"; break;
  default: mooseError("Unknown scheme"); break;
  }

  {
    InputParameters params = _app.getFactory().getValidParams(ti_str);
    _problem.addTimeIntegrator(ti_str, "ti", params);
  }
}
