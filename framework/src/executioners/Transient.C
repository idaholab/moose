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
#include "TimeScheme.h"
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
  MooseEnum schemes("backward-euler, implicit-euler, explicit-euler, crank-nicolson, bdf2, petsc", "backward-euler");

  params.addParam<Real>("start_time",      0.0,    "The start time of the simulation");
  params.addParam<Real>("end_time",        1.0e30, "The end time of the simulation");
  params.addRequiredParam<Real>("dt", "The timestep size between solves");
  params.addParam<Real>("dtmin",           0.0,    "The minimum timestep size in an adaptive run");
  params.addParam<Real>("dtmax",           1.0e30, "The maximum timestep size in an adaptive run");
  params.addParam<Real>("num_steps",       std::numeric_limits<Real>::max(),     "The number of timesteps in a transient run");
  params.addParam<int> ("n_startup_steps", 0,      "The number of timesteps during startup");
  params.addParam<bool>("trans_ss_check",  false,  "Whether or not to check for steady state conditions");
  params.addParam<Real>("ss_check_tol",    1.0e-08,"Whenever the relative residual changes by less than this the solution will be considered to be at steady state.");
  params.addParam<Real>("ss_tmin",         0.0,    "Minimum number of timesteps to take before checking for steady state conditions.");
  params.addParam<std::vector<Real> >("sync_times", sync_times, "A list of times that will be solved for provided they are within the simulation time");
  params.addParam<std::vector<Real> >("time_t", "The values of t");
  params.addParam<std::vector<Real> >("time_dt", "The values of dt");
  params.addParam<Real>("growth_factor", 2, "Maximum ratio of new to previous timestep sizes following a step that required the time step to be cut due to a failed solve.  For use with 'time_t' and 'time_dt'.");
  params.addParam<Real>("predictor_scale", "The scale factor for the predictor (can range from 0 to 1)");

  params.addParam<std::vector<std::string> >("time_periods", "The names of periods");
  params.addParam<std::vector<Real> >("time_period_starts", "The start times of time periods");
  params.addParam<std::vector<Real> >("time_period_ends", "The end times of time periods");
  params.addParam<bool>("use_AB2", false, "Whether to use the Adams-Bashforth 2 predictor");
  params.addParam<bool>("use_littlef", false, "if a function evaluation should be used or time deriv's in predictors");
  params.addParam<bool>("abort_on_solve_fail", false, "abort if solve not converged rather than cut timestep");
  params.addParam<MooseEnum>("scheme",          schemes,  "Time integration scheme used.");

  params.addParamNamesToGroup("start_time dtmin dtmax n_startup_steps trans_ss_check ss_check_tol ss_tmin sync_times time_t time_dt growth_factor predictor_scale use_AB2 use_littlef abort_on_solve_fail", "Advanced");

  params.addParamNamesToGroup("time_periods time_period_starts time_period_ends", "Time Periods");

  return params;
}

Transient::Transient(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(*getParam<FEProblem *>("_fe_problem")),
    _t_step(_problem.timeStep()),
    _time(_problem.time()),
    _time_old(_problem.timeOld()),
    _input_dt(getParam<Real>("dt")),
    _dt(_problem.dt()),
    _dt_old(_problem.dtOld()),
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
    _converged(true),
    _sync_times(getParam<std::vector<Real> >("sync_times")),
    _remaining_sync_time(true),
    _time_ipol(getParam<std::vector<Real> >("time_t"),
               getParam<std::vector<Real> >("time_dt")),
    _use_time_ipol(_time_ipol.getSampleSize() > 0),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_occurred(false),
    _abort(getParam<bool>("abort_on_solve_fail"))
{
  _t_step = 0;
  _dt = 0;
  _time = _time_old = getParam<Real>("start_time");
  _problem.transient(true);
  if (parameters.isParamValid("predictor_scale"))
  {
    Real predscale(getParam<Real>("predictor_scale"));
    if (predscale >= 0.0 and predscale <= 1.0)
    {
      _problem.getNonlinearSystem().setPredictorScale(getParam<Real>("predictor_scale"));
    }
    else
    {
      mooseError("Input value for predictor_scale = "<< predscale << ", outside of permissible range (0 to 1)");
    }
  }

  _problem.getTimeScheme()->_use_AB2 = getParam<bool>("use_AB2");
  _problem.getTimeScheme()->_use_littlef = getParam<bool>("use_littlef");
  if (!_restart_file_base.empty())
    _problem.setRestartFile(_restart_file_base);

  _problem.getNonlinearSystem().timeSteppingScheme(Moose::stringToEnum<Moose::TimeSteppingScheme>(getParam<MooseEnum>("scheme")));
}

Transient::~Transient()
{
  // This problem was built by the Factory and needs to be released by this destructor
  delete &_problem;
}

void
Transient::execute()
{
  _problem.initialSetup();

  preExecute();

  // NOTE: if you remove this line, you will see a subset of tests failing. Those tests might have a wrong answer and might need to be regolded.
  // The reason is that we actually move the solution back in time before we actually start solving (which I think is wrong).  So this call here
  // is to maintain backward compatibility and so that MOOSE is giving the same answer.  However, we might remove this call and regold the test
  // in the future eventually.
  _problem.copyOldSolutions();

  // Start time loop...
  while(keepGoing())
  {
    takeStep();
    endStep();
  }
  postExecute();
}

void
Transient::takeStep(Real input_dt)
{
  _dt_old = _dt;
  if (input_dt == -1.0)
    _dt = computeConstrainedDT();
  else
    _dt = input_dt;

  _problem.onTimestepBegin();
  if (_converged)
  {
    // Update backward material data structures
    _problem.updateMaterials();
  }

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

  _problem.timestepSetup();

  // Compute Pre-Aux User Objects (Timestep begin)
  _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN, UserObjectWarehouse::PRE_AUX);

  // Compute TimestepBegin AuxKernels
  _problem.computeAuxiliaryKernels(EXEC_TIMESTEP_BEGIN);

  // Compute Post-Aux User Objects (Timestep begin)
  _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN, UserObjectWarehouse::POST_AUX);

  _problem.solve();

  _converged = _problem.converged();

  // We know whether or not the nonlinear solver thinks it converged, but we need to see if the executioner concurs
  bool last_solve_converged = lastSolveConverged();

  if(last_solve_converged)
    std::cout<<"Solve Converged!"<<std::endl;
  else
    std::cout<<"Solve Did NOT Converge!"<<std::endl;

  if (last_solve_converged)
    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);

  // User definable callback
  postSolve();

  _problem.onTimestepEnd();

  if (last_solve_converged)
  {
    _problem.computeAuxiliaryKernels(EXEC_TIMESTEP);
    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::POST_AUX);
    _problem.execTransfers(EXEC_TIMESTEP);
    _problem.execMultiApps(EXEC_TIMESTEP);
  }
}

void
Transient::endStep()
{
  if (lastSolveConverged())
  {
    // Compute the Error Indicators and Markers
    _problem.computeIndicatorsAndMarkers();

    // if _reset_dt is true, force the output no matter what
    _problem.output(_reset_dt);
    _problem.outputPostprocessors(_reset_dt);

#ifdef LIBMESH_ENABLE_AMR
    if (_problem.adaptivity().isOn())
    {
      _problem.adaptMesh();
      _problem.out().meshChanged();
    }
#endif

    _time_old = _time;
    _t_step++;

    _problem.copyOldSolutions();
  }
  else
    _problem.restoreSolutions();
}

Real
Transient::computeConstrainedDT()
{
  // If this is the first step
  if (_t_step == 0)
  {
    _t_step = 1;
    _dt = _input_dt;
  }

//  // If start up steps are needed
//  if(_t_step == 1 && _n_startup_steps > 1)
//    _dt = _input_dt/(double)(_n_startup_steps);
//  else if (_t_step == 1+_n_startup_steps && _n_startup_steps > 1)
//    _dt = _input_dt;

  Real dt_cur = _dt;
  //After startup steps, compute new dt
  if (_t_step > _n_startup_steps)
    dt_cur = computeDT();

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
  if (_remaining_sync_time && _time + dt_cur >= *_curr_sync_time_iter)
  {
    dt_cur = *_curr_sync_time_iter - _time;
    if (++_curr_sync_time_iter == _sync_times.end())
      _remaining_sync_time = false;

    _prev_dt = _dt;

    _reset_dt = true;
  }
  else
  {
    if (_reset_dt)
    {
      if (_use_time_ipol)
        dt_cur = _time_ipol.sample(_time);
      else
        dt_cur = _prev_dt;
      _reset_dt = false;
    }
  }

  // Constrain by what the multi apps are doing
  Real multi_app_dt = _problem.computeMultiAppsDT(EXEC_TIMESTEP_BEGIN);
  if(multi_app_dt < dt_cur)
    dt_cur = multi_app_dt;

  return dt_cur;

}

Real
Transient::computeDT()
{
  if (!lastSolveConverged())
  {
    _cutback_occurred = true;
    //std::cout<<"Solve failed... cutting timestep"<<std::endl;
    //return _dt / 2.0;

    // Instead of blindly cutting timestep, respect dtmin.
    if (_dt <= _dtmin)
      mooseError("Solve failed and timestep already at or below dtmin, cannot continue!");

    if (0.5 * _dt >= _dtmin)
      return 0.5 * _dt;

    else // (0.5 * _dt < _dtmin)
      return _dtmin;
  }

  Real dt = _dt;
  if (_use_time_ipol)
  {
    dt = _time_ipol.sample(_time);
    if (_cutback_occurred &&
        dt > _dt * _growth_factor)
    {
      dt = _dt * _growth_factor;
    }
  }

  _cutback_occurred = false;
  return dt;
}

bool
Transient::keepGoing()
{
  // Check for stop condition based upon steady-state check flag:
  if(_converged && _trans_ss_check == true && _time > _ss_tmin)
  {
    // Compute new time solution l2_norm
    Real new_time_solution_norm = _problem.getNonlinearSystem().currentSolution()->l2_norm();

    // Compute l2_norm relative error
    Real ss_relerr_norm = fabs(new_time_solution_norm - _old_time_solution_norm)/new_time_solution_norm;

    // Check current solution relative error norm against steady-state tolerance
    if(ss_relerr_norm < _ss_check_tol)
    {
      std::cout<<"Steady-State Solution Achieved at time: "<<_time<<std::endl;
      return false;
    }
    else // Keep going
    {
      // Update solution norm for next time step
      _old_time_solution_norm = new_time_solution_norm;
      // Print steady-state relative error norm
      std::cout<<"Steady-State Relative Differential Norm: "<<ss_relerr_norm<<std::endl;
    }
  }

  if(!_converged && _abort)
  {
    std::cout<<"Aborting as solve did not converge and input selected to abort"<<std::endl;
    return false;
  }

  // Check for stop condition based upon number of simulation steps and/or solution end time:
  if(_t_step>_num_steps)
    return false;

  if((_time>_end_time) || (fabs(_time-_end_time)<1.e-14))
    return false;

  return true;
}


bool
Transient::lastSolveConverged()
{
  return _converged;
}

void
Transient::preExecute()
{
  // process time periods
  const std::vector<TimePeriod *> _time_periods = _problem.getTimePeriods();
  for (unsigned int i = 0; i < _time_periods.size(); ++i)
    _sync_times.push_back(_time_periods[i]->start());

  const std::vector<Real> & time = getParam<std::vector<Real> >("time_t");
  if (_use_time_ipol)
    _sync_times.insert(_sync_times.end(), time.begin()+1, time.end());          // insert times as sync points except the very first one
  sort(_sync_times.begin(), _sync_times.end());
  _sync_times.erase(std::unique(_sync_times.begin(), _sync_times.end()), _sync_times.end());    // remove duplicates (needs sorted array)

  // Advance to the first sync time if one is provided in sim time range
  _curr_sync_time_iter = _sync_times.begin();
  while (_remaining_sync_time && *_curr_sync_time_iter <= _time)
    if (++_curr_sync_time_iter == _sync_times.end())
      _remaining_sync_time = false;
}

Problem &
Transient::problem()
{
  return _problem;
}
