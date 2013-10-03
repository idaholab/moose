#include "AdaptiveDT.h"

#include "Function.h"

template<>
InputParameters validParams<AdaptiveDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<int>("optimal_iterations", "The target number of nonlinear iterations for adaptive timestepping");
  params.addParam<int>("iteration_window",  "The size of the nonlinear iteration window for adaptive timestepping (default = 0.2*optimal_iterations)");
  params.addParam<unsigned> ("linear_iteration_ratio", "The ratio of linear to nonlinear iterations to determine target linear iterations and window for adaptive timestepping (default = 25)");
  params.addParam<FunctionName> ("timestep_limiting_function", "A function used to control the timestep by limiting the change in the function over a timestep");
  params.addParam<Real> ("max_function_change", "The absolute value of the maximum change in timestep_limiting_function over a timestep");
  params.addRequiredParam<Real>("dt", "The default timestep size between solves");
  params.addParam<std::vector<Real> >("time_t", "The values of t");
  params.addParam<std::vector<Real> >("time_dt", "The values of dt");
  params.addParam<Real>("growth_factor",   2, "Factor to apply to timestep if easy convergence (if 'optimal_iterations' is specified) or if recovering from failed solve");
  params.addParam<Real>("cutback_factor",  0.5, "Factor to apply to timestep if difficult convergence (if 'optimal_iterations' is specified) or if solution failed.");

  return params;
}

AdaptiveDT::AdaptiveDT(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters),
    _input_dt(getParam<Real>("dt")),
    _synced_last_step(false),
    _linear_iteration_ratio(isParamValid("linear_iteration_ratio") ? getParam<unsigned>("linear_iteration_ratio") : 25),  // Default to 25
    _adaptive_timestepping(false),
    _timestep_limiting_function(NULL),
    _max_function_change(-1),
    _sync_times(getParam<std::vector<Real> >("time_t")),
    _sync_times_iter(_sync_times.begin()),
    _remaining_sync_time(_sync_times.size() > 0),
    _time_ipol(getParam<std::vector<Real> >("time_t"),
               getParam<std::vector<Real> >("time_dt")),
    _use_time_ipol(_time_ipol.getSampleSize() > 0),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_factor(getParam<Real>("cutback_factor"))
{

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
    if (isParamValid("linear_iteration_ratio"))
    {
      mooseError("'optimal_iterations' must be used for 'linear_iteration_ratio' to be used");
    }
  }

  if (isParamValid("timestep_limiting_function"))
  {
    _max_function_change = isParamValid("max_function_change") ?
        getParam<Real>("max_function_change") : -1;
    if (_max_function_change <= 0.0)
    {
      mooseError("'max_function_change' must be greater than 0");
    }
  }
  else
  {
    if (isParamValid("max_function_change"))
    {
      mooseError("'timestep_limiting_function' must be used for 'max_function_change' to be used");
    }
  }

  // Advance to the first sync time if one is provided in sim time range
  while (_remaining_sync_time && *_sync_times_iter <= _time)
  {
    if (++_sync_times_iter == _sync_times.end())
    {
      _remaining_sync_time = false;
    }
  }

}

AdaptiveDT::~AdaptiveDT()
{
}

Real
AdaptiveDT::computeInitialDT()
{
  // Advance to the first sync time if one is provided in sim time range
  _sync_times_iter = _sync_times.begin();
  _remaining_sync_time = _sync_times_iter != _sync_times.end();
  while (_remaining_sync_time && *_sync_times_iter <= _time)
  {
    if (++_sync_times_iter == _sync_times.end())
    {
      _remaining_sync_time = false;
    }
  }

  if (isParamValid("timestep_limiting_function"))
  {
    _timestep_limiting_function = &_fe_problem.getFunction(getParam<FunctionName>("timestep_limiting_function"), isParamValid("_tid") ? getParam<THREAD_ID>("_tid") : 0);
  }

  Real dt = _input_dt;
  limitDTByFunction(dt);

  // Adjust to the next sync time if needed
  if (_remaining_sync_time && _time_old + dt >= *_sync_times_iter)
  {
    _synced_last_step = true;

    dt = *_sync_times_iter - _time_old;

    std::cout << "Limiting dt to sync with dt function time: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << *_sync_times_iter
              << " dt: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << dt
              << std::endl;

    if (++_sync_times_iter == _sync_times.end())
    {
      _remaining_sync_time = false;
    }

  }

  return dt;
}

Real
AdaptiveDT::computeDT()
{
  Real dt(_dt);

  if (_synced_last_step)
  {
    _synced_last_step = false;

    dt = _time_ipol.sample(_time_old);

    std::cout << "Setting dt to value specified by dt function: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << dt
              << std::endl;
  }
  else if (_adaptive_timestepping)
  {
    computeAdaptiveDT(dt);
  }
  else if (_use_time_ipol)
  {
    computeInterpolationDT(dt);
  }
  else
  {
    dt *= _growth_factor;
    if (dt > _dt * _growth_factor)
    {
      dt = _dt * _growth_factor;
    }
  }

  // Limit the timestep to limit change in the function
  limitDTByFunction(dt);

  // Adjust to the next sync time if needed
  if (_remaining_sync_time && _time_old + dt >= *_sync_times_iter)
  {
    _synced_last_step = true;

    dt = *_sync_times_iter - _time_old;

    std::cout << "Limiting dt to sync with dt function time: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << *_sync_times_iter
              << " dt: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << dt
              << std::endl;

    if (++_sync_times_iter == _sync_times.end())
    {
      _remaining_sync_time = false;
    }

  }

  return dt;
}

Real
AdaptiveDT::computeFailedDT()
{
  Real dt(_dt);

  if (dt <= _dt_min)
  { //Can't cut back any more
    mooseError("Solve failed and timestep already at dtmin, cannot continue!");
  }

  dt = _cutback_factor * _dt;
  if (dt < _dt_min)
  {
    dt =  _dt_min;
  }

  if (_adaptive_timestepping)
  {
    bool allowToGrow = false;
    computeAdaptiveDT(dt, allowToGrow);
  }

  // Limit the timestep to limit change in the function
  //TODO: handle conflict of this with dtmin
  limitDTByFunction(dt);
  return dt;
}

void
AdaptiveDT::limitDTByFunction(Real & limitedDT)
{
  if (_timestep_limiting_function)
  {
    Point dummyPoint;
    Real oldValue = _timestep_limiting_function->value(_time_old,dummyPoint);
    Real newValue = _timestep_limiting_function->value(_time_old+limitedDT,dummyPoint);
    Real change = std::abs(newValue-oldValue);
    if (change > _max_function_change)
    {
      do
      {
        limitedDT /= 2.0;
        newValue = _timestep_limiting_function->value(_time_old+limitedDT,dummyPoint);
        change = std::abs(newValue-oldValue);
      }
      while (change > _max_function_change);
    }
  }
  if (limitedDT < _dt_min)
  {
    limitedDT = _dt_min;
  }
}

void
AdaptiveDT::computeAdaptiveDT(Real &dt, bool allowToGrow, bool allowToShrink)
{
  const unsigned int nl_its = _fe_problem.getNonlinearSystem().nNonlinearIterations();
  const unsigned int l_its = _fe_problem.getNonlinearSystem().nLinearIterations();
  const unsigned int growth_nl_its(_optimal_iterations > _iteration_window ? _optimal_iterations - _iteration_window : 0);
  const unsigned int shrink_nl_its(_optimal_iterations + _iteration_window);
  const unsigned int growth_l_its(_optimal_iterations > _iteration_window ? _linear_iteration_ratio*(_optimal_iterations - _iteration_window) : 0);
  const unsigned int shrink_l_its(_linear_iteration_ratio*(_optimal_iterations + _iteration_window));

  if (allowToGrow && (nl_its < growth_nl_its && l_its < growth_l_its))
  { //grow the timestep
    dt *= _growth_factor;

    std::cout << "Growing dt: nl its = "<<nl_its<<" < "<<growth_nl_its
              << " && lin its = "<<l_its<<" < "<<growth_l_its
              << " old dt: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << _dt
              << " new dt: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << dt
              << std::endl;
  }
  else if (allowToShrink && (nl_its > shrink_nl_its || l_its > shrink_l_its))
  { //shrink the timestep
    dt *= _cutback_factor;

    std::cout << "Shrinking dt: nl its = "<<nl_its<<" > "<<shrink_nl_its
              << " || lin its = "<<l_its<<" > "<<shrink_l_its
              << " old dt: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << _dt
              << " new dt: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << dt
              << std::endl;
  }
}

void
AdaptiveDT::computeInterpolationDT(Real & dt)
{
  dt = _time_ipol.sample(_time_old);
  if (dt > _dt * _growth_factor)
  {
    dt = _dt * _growth_factor;

    std::cout << "Growing dt to recover from cutback.  old dt: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << _dt
              << " new dt: "
              << std::setw(9)
              << std::setprecision(6)
              << std::setfill('0')
              << std::showpoint
              << std::left
              << dt
              << std::endl;
  }
}


void
AdaptiveDT::rejectStep()
{
  std::cout<<"Solve failed... cutting timestep"<<std::endl;

  TimeStepper::rejectStep();
}
