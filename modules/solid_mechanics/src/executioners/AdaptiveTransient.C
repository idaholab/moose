
#include "AdaptiveTransient.h"

//Moose includes
#include "Kernel.h"
#include "Factory.h"
#include "SubProblem.h"
#include "ProblemFactory.h"
#include "Function.h"

//libMesh includes
#include "implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "numeric_vector.h"

// C++ Includes
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>


bool sortsyncpair(const std::pair<Real,AdaptiveTransient::SyncType> &a, const std::pair<Real,AdaptiveTransient::SyncType> &b) { return (a.first < b.first); }

template<>
InputParameters validParams<AdaptiveTransient>()
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
  params.addParam<Real>("growth_factor",   2, "Factor to apply to timestep if easy convergence (if 'optimal_iterations' is specified) or if recovering from failed solve");
  params.addParam<Real>("cutback_factor",  0.5, "Factor to apply to timestep if difficult convergence (if 'optimal_iterations' is specified) or if solution failed.");
  params.addParam<Real>("predictor_scale", "The scale factor for the predictor (can range from 0 to 1)");
  params.addParam<int> ("optimal_iterations", "The target number of nonlinear iterations for adaptive timestepping");
  params.addParam<int> ("iteration_window",  "The size of the nonlinear iteration window for adaptive timestepping (default = 0.2*optimal_iterations)");
  params.addParam<int> ("linear_iteration_ratio", 25, "The ratio of linear to nonlinear iterations to determine target linear iterations and window for adaptive timestepping (default = 25)");
  params.addParam<std::string> ("timestep_limiting_function", "A function used to control the timestep by limiting the change in the function over a timestep");
  params.addParam<Real> ("max_function_change", "The absolute value of the maximum change in timestep_limiting_function over a timestep");

  return params;
}

AdaptiveTransient::AdaptiveTransient(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(*getParam<FEProblem *>("_fe_problem")),
    _t_step(_problem.timeStep()),
    _time(_problem.time()),
    _time_old(_time),
    _input_dt(getParam<Real>("dt")),
    _dt(_problem.dt()),
    _dt_old(_problem.dtOld()),
    _prev_dt(-1),
    _synced_last_step(false),
    _end_time(getParam<Real>("end_time")),
    _dtmin(getParam<Real>("dtmin")),
    _dtmax(getParam<Real>("dtmax")),
    _num_steps(getParam<Real>("num_steps")),
    _n_startup_steps(getParam<int>("n_startup_steps")),
    _linear_iteration_ratio(getParam<int>("linear_iteration_ratio")),
    _adaptive_timestepping(false),
    _timestep_limiting_function(NULL),
    _max_function_change(-1.0),
    _trans_ss_check(getParam<bool>("trans_ss_check")),
    _ss_check_tol(getParam<Real>("ss_check_tol")),
    _ss_tmin(getParam<Real>("ss_tmin")),
    _converged(true),
    _caught_exception(false),
    _remaining_sync_time(true),
    _time_ipol(getParam<std::vector<Real> >("time_t"),
               getParam<std::vector<Real> >("time_dt")),
    _use_time_ipol(_time_ipol.getSampleSize() > 0),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_factor(getParam<Real>("cutback_factor")),
    _cutback_occurred(false)
{
  _t_step = 0;
  _dt = 0;
  _time = getParam<Real>("start_time");
  _problem.transient(true);
  if (isParamValid("predictor_scale"))
  {
    Real predscale(getParam<Real>("predictor_scale"));
    if (predscale >= 0.0 and predscale <= 1.0)
      _problem.getNonlinearSystem().setPredictorScale(predscale);
    else
      mooseError("Input value for predictor_scale = "<< predscale << ", outside of permissible range (0 to 1)");
  }

  if (isParamValid("optimal_iterations"))
  {
    _adaptive_timestepping=true;
    _optimal_iterations = getParam<int>("optimal_iterations");
    if (isParamValid("iteration_window"))
    {
      _iteration_window = getParam<int>("iteration_window");
    }
    else
    {
      _iteration_window = ceil(_optimal_iterations/5.0);
    }
  }
  else
  {
    if (isParamValid("iteration_window"))
    {
      mooseError("'optimal_iterations' must be used for 'iteration_window' to be used");
    }
    if (parameters.wasSeenInInput("linear_iteration_ratio"))
    {
      mooseError("'optimal_iterations' must be used for 'linear_iteration_ratio' to be used");
    }
  }

  if (isParamValid("timestep_limiting_function"))
  {
    _timestep_limiting_function_name = getParam<std::string>("timestep_limiting_function");
    //TODO:  It would be nice to grab the function here, but it doesn't seem to exist yet.  This doesn't work:
    //_timestep_limiting_function = &_problem.getFunction(_timestep_limiting_function_name);
    _max_function_change = isParamValid("max_function_change") ?
        getParam<Real>("max_function_change") : -1;
    if (_max_function_change < 0.0)
    {
      mooseError("'max_function_change' must be specified and be greater than 0");
    }
  }
  else
  {
    if (isParamValid("max_function_change"))
    {
      mooseError("'timestep_limiting_function' must be used for 'max_function_change' to be used");
    }
  }

  const std::vector<Real> & sync_times = getParam<std::vector<Real> >("sync_times");
  for (unsigned int i(0); i<sync_times.size(); ++i)
  {
    std::pair<Real,SyncType> st_pair = std::make_pair(sync_times[i],SYNC);
    _sync_times.push_back(st_pair);
  }
  const std::vector<Real> & time = getParam<std::vector<Real> >("time_t");
  if (_use_time_ipol)
  {
    for (unsigned int i(0); i<time.size(); ++i)
    {
      std::pair<Real,SyncType> st_pair = std::make_pair(time[i],TIME_FUNC);
      _sync_times.push_back(st_pair);
    }
  }
  std::sort(_sync_times.begin(), _sync_times.end(),sortsyncpair);
  _curr_sync_time_iter = _sync_times.begin();

  // Advance to the first sync time if one is provided in sim time range
  while (_remaining_sync_time && _curr_sync_time_iter->first <= _time)
  {
    if (++_curr_sync_time_iter == _sync_times.end())
    {
      _remaining_sync_time = false;
    }
  }

  if (!_restart_file_base.empty())
    _problem.setRestartFile(_restart_file_base);
}

AdaptiveTransient::~AdaptiveTransient()
{
  // This problem was built by the Factory and needs to be released by this destructor
  delete &_problem;
}

void
AdaptiveTransient::execute()
{
  if (_timestep_limiting_function_name != "")
    _timestep_limiting_function = &_problem.getFunction(_timestep_limiting_function_name);
  _problem.initialSetup();

  preExecute();

  // Start time loop...
  while(keepGoing())
  {
    takeStep();
    if (lastSolveConverged())
      endStep();
  }
  postExecute();
}

void
AdaptiveTransient::takeStep(Real input_dt)
{
  if (_converged)
    _problem.copyOldSolutions();
  else
    _problem.restoreSolutions();

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

  // Compute TimestepBegin AuxKernels
  _problem.computeAuxiliaryKernels(EXEC_TIMESTEP_BEGIN);

  // Compute TimestepBegin Postprocessors
  _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN);

  std::cout << "Solving time step ";
  {
    OStringStream out;
    OSSInt(out,2,_t_step);
    out << ", old time=";
    OSSRealzeroleft(out,9,6,_time_old);
    out << ", time=";
    OSSRealzeroleft(out,9,6,_time);
    out << ", dt=";
    OSSRealzeroleft(out,9,6,_dt);
    std::cout << out.str() << std::endl;
  }

  preSolve();

  _problem.timestepSetup();

  try
  {
    // Compute TimestepBegin AuxKernels
    _problem.computeAuxiliaryKernels(EXEC_TIMESTEP_BEGIN);

    // Compute TimestepBegin Postprocessors
    _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN);

    _problem.solve();
    _converged = _problem.converged();
  }
  catch (MooseException &e)
  {
    _caught_exception = true;
    _converged = false;
  }

  if (!_caught_exception)
  {
    postSolve();

    _problem.onTimestepEnd();

    // We know whether or not the nonlinear solver thinks it converged, but we need to see if the executioner concurs
    if (lastSolveConverged())
      _problem.computeUserObjects();

    std::cout << std::endl;
  }
}

void
AdaptiveTransient::endStep()
{
  // if _synced_last_step is true, force the output no matter what
  _problem.output(_synced_last_step);
  _problem.outputPostprocessors(_synced_last_step);

#ifdef LIBMESH_ENABLE_AMR
  if (_problem.adaptivity().isOn())
  {
    _problem.adaptMesh();
    _problem.out().meshChanged();
  }
#endif

  _t_step++;
  _time_old = _time;
}

Real
AdaptiveTransient::computeConstrainedDT()
{
  _diag.str("");
  _diag.clear();

  // If this is the first step
  if (_t_step == 0)
  {
    _t_step = 1;
    _dt = _input_dt;
  }

  Real dt_cur = computeDT();

  // Don't let the time step size exceed maximum time step size
  if (dt_cur > _dtmax)
  {
    dt_cur = _dtmax;

    _diag << "Limiting dt to dtmax: ";
    OSSRealzeroleft(_diag,9,6,_dtmax);
    _diag << std::endl;
  }

  // Limit the timestep to limit change in the function
  //TODO: handle conflict of this with dtmin
  Real function_limited_dt = limitDTByFunction(dt_cur);
  if (function_limited_dt < dt_cur)
  {
    dt_cur = function_limited_dt;
    _diag << "Limiting dt to limit change in function. dt: ";
    OSSRealzeroleft(_diag,9,6,dt_cur);
    _diag << std::endl;
  }

  // Don't allow time step size to be smaller than minimum time step size
  if (dt_cur < _dtmin)
  {
    dt_cur = _dtmin;

    _diag << "Increasing dt to dtmin: ";
    OSSRealzeroleft(_diag,9,6,_dtmin);
    _diag << std::endl;
  }

  // Don't let time go beyond simulation end time
  if (_time_old + dt_cur > _end_time)
  {
    dt_cur = _end_time - _time_old;

    _diag << "Limiting dt for end time: ";
    OSSRealzeroleft(_diag,9,6,_end_time);
    _diag << " dt: ";
    OSSRealzeroleft(_diag,9,6,dt_cur);
    _diag << std::endl;
  }

  // Adjust to the next sync time if needed
  if (_remaining_sync_time && _time_old + dt_cur >= _curr_sync_time_iter->first)
  {
    dt_cur = _curr_sync_time_iter->first - _time_old;

    _synced_last_step = true;
    _last_sync_type = _curr_sync_time_iter->second;

    if (_last_sync_type == SYNC)
      _diag << "Limiting dt for sync time: ";
    else
      _diag << "Limiting dt to sync with dt function time: ";
    OSSRealzeroleft(_diag,9,6,_curr_sync_time_iter->first);
    _diag << " dt: ";
    OSSRealzeroleft(_diag,9,6,dt_cur);
    _diag << std::endl;

    if (++_curr_sync_time_iter == _sync_times.end())
      _remaining_sync_time = false;

    _prev_dt = _dt;

  }

  if (_diag.str().size() > 0)
  {
    std::cout << _diag.str() << std::endl;
  }

  return dt_cur;

}

Real
AdaptiveTransient::computeDT()
{
  Real dt = _dt;

  if (!lastSolveConverged())
  {
    if (dt <= _dtmin)
    { //Can't cut back any more
      mooseError("Solve failed and timestep already at dtmin, cannot continue!");
    }

    dt = _cutback_factor * _dt;
    if (dt < _dtmin)
    {
      dt =  _dtmin;
    }

    if (_caught_exception)
    {
      _diag << "Encountered exception with dt: ";
      _caught_exception = false;
    }
    else
      _diag << "Solve failed with dt: ";
    OSSRealzeroleft(_diag,9,6,_dt);
    _diag << "  Retrying with reduced dt: ";
    OSSRealzeroleft(_diag,9,6,dt);
    _diag << std::endl;
  }

  else if ( _t_step == 1 ) //First time step, don't change the timestep ...
  {
  }

  else if ( _t_step <= _n_startup_steps ) //Don't change the timestep ...
  {
    if (_synced_last_step) //  ... unless we did a sync during the startup steps
    {
      dt = _prev_dt;

      if (_last_sync_type == SYNC)
        _diag << "Within n_startup_steps, resetting dt to value used before sync: ";
      else
        _diag << "Within n_startup_steps, resetting dt to value used before syncing with dt function: ";
      OSSRealzeroleft(_diag,9,6,dt);
      _diag << std::endl;
    }
    else
    {
      _diag << "Within n_startup_steps, maintaining dt : ";
      OSSRealzeroleft(_diag,9,6,dt);
      _diag << std::endl;
    }
  }

  else if ( _cutback_occurred ) //Don't allow it to grow this step, but shrink if needed
  {
    if (_adaptive_timestepping)
    {
      bool allowToGrow(false);
      computeAdaptiveDT(dt,allowToGrow);
    }
  }

  else if (_synced_last_step)
  {
    if (_last_sync_type == SYNC)
    {
      dt = _prev_dt;
      _diag << "Resetting dt to value used before sync: ";
    }
    else if (_last_sync_type == TIME_FUNC)
    {
      if (!_use_time_ipol)
        mooseError("Can't have TIME_FUNC sync type without time_ipol");
      dt = _time_ipol.sample(_time_old);
      _diag << "Setting dt to value specified by dt function: ";
    }

    OSSRealzeroleft(_diag,9,6,dt);
    _diag << std::endl;
  }

  else
  {
    if (_adaptive_timestepping)
    {
      computeAdaptiveDT(dt);
    }
    else
    {
      if (_use_time_ipol)
      {
        dt = _time_ipol.sample(_time_old);
        if (dt > _dt * _growth_factor)
        {
          dt = _dt * _growth_factor;

          _diag << "Growing dt to recover from cutback.  old dt: ";
          OSSRealzeroleft(_diag,9,6,_dt);
          _diag << " new dt: ";
          OSSRealzeroleft(_diag,9,6,dt);
          _diag << std::endl;
        }
      }
      else
      {
        dt = _input_dt;
        if (dt > _dt * _growth_factor)
        {
          dt = _dt * _growth_factor;

          _diag << "Growing dt to recover from cutback.  old dt: ";
          OSSRealzeroleft(_diag,9,6,_dt);
          _diag << " new dt: ";
          OSSRealzeroleft(_diag,9,6,dt);
          _diag << std::endl;
        }
      }
    }
  }

  if (!lastSolveConverged())
    _cutback_occurred = true;
  else
    _cutback_occurred = false;

  _synced_last_step = false;

  return dt;
}

void
AdaptiveTransient::computeAdaptiveDT(Real &dt, bool allowToGrow, bool allowToShrink)
{
  unsigned int nl_its = _problem.getNonlinearSystem().nNonlinearIterations();
  unsigned int l_its = _problem.getNonlinearSystem().nLinearIterations();
  unsigned int growth_nl_its(_optimal_iterations - _iteration_window);
  unsigned int shrink_nl_its(_optimal_iterations + _iteration_window);
  unsigned int growth_l_its(_linear_iteration_ratio*(_optimal_iterations - _iteration_window));
  unsigned int shrink_l_its(_linear_iteration_ratio*(_optimal_iterations + _iteration_window));

  if (nl_its < growth_nl_its && l_its < growth_l_its)
  { //grow the timestep
    if (allowToGrow){
      dt = _dt * _growth_factor;

      _diag << "Growing dt: nl its = "<<nl_its<<" < "<<growth_nl_its
            << " && lin its = "<<l_its<<" < "<<growth_l_its;
      _diag << " old dt: ";
      OSSRealzeroleft(_diag,9,6,_dt);
      _diag << " new dt: ";
      OSSRealzeroleft(_diag,9,6,dt);
      _diag << std::endl;
    }
  }
  else if (nl_its > shrink_nl_its || l_its > shrink_l_its)
  { //shrink the timestep
    if (allowToShrink){
      dt = _dt * _cutback_factor;

      _diag << "Shrinking dt: nl its = "<<nl_its<<" > "<<shrink_nl_its
            << " || lin its = "<<l_its<<" > "<<shrink_l_its;
      _diag << " old dt: ";
      OSSRealzeroleft(_diag,9,6,_dt);
      _diag << " new dt: ";
      OSSRealzeroleft(_diag,9,6,dt);
      _diag << std::endl;
    }
  }
}

Real
AdaptiveTransient::limitDTByFunction(Real trialDt)
{
  Real limitedDt = trialDt;
  if (_timestep_limiting_function)
  {
    Point dummyPoint;
    Real oldValue = _timestep_limiting_function->value(_time_old,dummyPoint);
    Real newValue = _timestep_limiting_function->value(_time_old+limitedDt,dummyPoint);
    Real change = std::abs(newValue-oldValue);
    if (change > _max_function_change)
    {
      do
      {
        limitedDt /= 2.0;
        newValue = _timestep_limiting_function->value(_time_old+limitedDt,dummyPoint);
        change = std::abs(newValue-oldValue);
      }
      while (change > _max_function_change);
    }
  }
  return limitedDt;
}

bool
AdaptiveTransient::keepGoing()
{
  bool kg = true;
  // Check for stop condition based upon number of simulation steps and/or solution end time:
  if(_t_step>_num_steps)
    kg = false;

  if((_time>_end_time) || (fabs(_time-_end_time)<1.e-14))
  {
    if (lastSolveConverged())
    {
      kg = false;
    }
  }

  return kg;
}


bool
AdaptiveTransient::lastSolveConverged()
{
  return _converged;
}

void
AdaptiveTransient::preExecute()
{
  Executioner::preExecute();
}
