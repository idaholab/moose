#include "Transient.h"

//Moose includes
#include "Kernel.h"
#include "Factory.h"
#include "SubProblem.h"

//libMesh includes
#include "implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "numeric_vector.h"

// C++ Includes
#include <iomanip>
#include <iostream>
#include <fstream>

template<>
InputParameters validParams<Transient>()
{
  InputParameters params = validParams<Executioner>();
  std::vector<Real> sync_times(1);
  sync_times[0] = -1;

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

  return params;
}

Transient::Transient(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(*_mesh),
    _t_step(_problem.timeStep()),
    _time(_problem.time()),
    _time_old(_time),
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
    _converged(true),
    _sync_times(getParam<std::vector<Real> >("sync_times")),
    _curr_sync_time_iter(_sync_times.begin()),
    _remaining_sync_time(true),
    _time_ipol(getParam<std::vector<Real> >("time_t"),
               getParam<std::vector<Real> >("time_dt")),
    _use_time_ipol(_time_ipol.getSampleSize() > 0)
{
  _t_step = 0;
  _dt = _input_dt;
  _time = getParam<Real>("start_time");
  _problem.transient(true);

  const std::vector<Real> & time = getParam<std::vector<Real> >("time_t");
  if (_use_time_ipol)
  {
    _sync_times.insert(_sync_times.end(), time.begin()+1, time.end());          // insert times as sync points except the very first one
    _curr_sync_time_iter = _sync_times.begin();
  }
  sort(_sync_times.begin(), _sync_times.end());

  // Advance to the first sync time if one is provided in sim time range
  while (_remaining_sync_time && *_curr_sync_time_iter < _time)
    if (++_curr_sync_time_iter == _sync_times.end())
      _remaining_sync_time = false;
}

Transient::~Transient()
{
}

void
Transient::execute()
{
  _problem.copySolutionsBackwards();
  _problem.adaptivity().initial();

  //Update the geometric searches (has to be called after the problem is all set up)
  _problem.geomSearchData().update();

  preExecute();
  _problem.update();
  _problem.computePostprocessors();
  if (_output_initial)
  {
    _problem.output();
    _problem.outputPostprocessors();
  }

  // Start time loop...
  while(keepGoing()) 
  {
    takeStep();
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

  // Increment time
  _time = _time_old + _dt;

  std::cout<<"DT: "<<_dt<<std::endl;

  std::cout << " Solving time step ";
  {
    OStringStream out;
      
    OSSInt(out,2,_t_step);
    out << ", time=";
    OSSRealzeroleft(out,9,6,_time);
    out <<  "...";
    std::cout << out.str() << std::endl;
  }
    
  preSolve();
    
  Moose::perf_log.push("solve()","Solve");
  // System Solve
  _problem.solve();
  Moose::perf_log.pop("solve()","Solve");

  _converged = _problem.converged();

  postSolve();

  _problem.onTimestepEnd();

  // We know whether or not the nonlinear solver thinks it converged, but we need to see if the executioner concurs
  bool last_solve_converged = lastSolveConverged();
    
  // If _reset_dt is true, the time step was synced to the user defined value and we dump the solution in an output file
  if (last_solve_converged) 
  {
    _problem.computePostprocessors();

    if (((_t_step + 1) % _problem.out().interval() == 0 || _reset_dt))
      _problem.output();

    _problem.outputPostprocessors();

    if (_problem.adaptivity().isOn())
    {
      _problem.adaptMesh();
      _problem.out().meshChanged();
    }

    _t_step++;
    _time_old = _time;
  }
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

  return dt_cur;
  
}

Real
Transient::computeDT()
{
  if (!lastSolveConverged())
  {
    std::cout<<"Solve failed... cutting timestep"<<std::endl;
    return _dt / 2.0;
  }
  
  if (_use_time_ipol)
  {
    return _time_ipol.sample(_time);
  }
  else
  {
//    // If start up steps are needed
//    if(_t_step == 1 && _n_startup_steps > 1)
//      return _dt/(double)(_n_startup_steps);
//    else if (_t_step == 1+_n_startup_steps && _n_startup_steps > 1)
//      return _dt*(double)(_n_startup_steps);
//    else
      return _dt;
  }
}

bool
Transient::keepGoing()
{
  // FIXME:
//  // Check for stop condition based upon steady-state check flag:
//  if(_converged && _trans_ss_check == true && _time > _ss_tmin)
//  {
//    // Compute new time solution l2_norm
//    Real new_time_solution_norm = _moose_system.getNonlinearSystem()->current_local_solution->l2_norm();
//
//    // Compute l2_norm relative error
//    Real ss_relerr_norm = fabs(new_time_solution_norm - _old_time_solution_norm)/new_time_solution_norm;
//
//    // Check current solution relative error norm against steady-state tolerance
//    if(ss_relerr_norm < _ss_check_tol)
//    {
//      std::cout<<"Steady-State Solution Achieved at time: "<<_time<<std::endl;
//      return false;
//    }
//    else // Keep going
//    {
//      // Update solution norm for next time step
//      _old_time_solution_norm = new_time_solution_norm;
//      // Print steady-state relative error norm
//      std::cout<<"Steady-State Relative Differential Norm: "<<ss_relerr_norm<<std::endl;
//    }
//  }

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

}

