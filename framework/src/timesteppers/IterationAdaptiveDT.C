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

// MOOSE includes
#include "IterationAdaptiveDT.h"
#include "Function.h"
#include "Piecewise.h"
#include "Transient.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<IterationAdaptiveDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addClassDescription("Adjust the timestep based on the number of iterations");
  params.addParam<int>("optimal_iterations",
                       "The target number of nonlinear iterations for adaptive timestepping");
  params.addParam<int>("iteration_window",
                       "Attempt to grow/shrink timestep if the iteration count "
                       "is below/above 'optimal_iterations plus/minus "
                       "iteration_window' (default = optimal_iterations/5).");
  params.addParam<unsigned>("linear_iteration_ratio",
                            "The ratio of linear to nonlinear iterations "
                            "to determine target linear iterations and "
                            "window for adaptive timestepping (default = "
                            "25)");
  params.addParam<PostprocessorName>("postprocessor_dtlim",
                                     "If specified, the postprocessor value "
                                     "is used as an upper limit for the "
                                     "current time step length");
  params.addParam<FunctionName>("timestep_limiting_function",
                                "A 'Piecewise' type function used to control the timestep by "
                                "limiting the change in the function over a timestep");
  params.addParam<Real>(
      "max_function_change",
      "The absolute value of the maximum change in timestep_limiting_function over a timestep");
  params.addParam<bool>("force_step_every_function_point",
                        false,
                        "Forces the timestepper to take "
                        "a step that is consistent with "
                        "points defined in the function");
  params.addRequiredParam<Real>("dt", "The default timestep size between solves");
  params.addParam<std::vector<Real>>("time_t", "The values of t");
  params.addParam<std::vector<Real>>("time_dt", "The values of dt");
  params.addParam<Real>("growth_factor",
                        2.0,
                        "Factor to apply to timestep if easy convergence (if "
                        "'optimal_iterations' is specified) or if recovering "
                        "from failed solve");
  params.addParam<Real>("cutback_factor",
                        0.5,
                        "Factor to apply to timestep if difficult "
                        "convergence (if 'optimal_iterations' is specified) "
                        "or if solution failed");
  return params;
}

IterationAdaptiveDT::IterationAdaptiveDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    PostprocessorInterface(this),
    _dt_old(declareRestartableData<Real>("dt_old", 0.0)),
    _input_dt(getParam<Real>("dt")),
    _tfunc_last_step(declareRestartableData<bool>("tfunc_last_step", false)),
    _sync_last_step(declareRestartableData<bool>("sync_last_step", false)),
    _linear_iteration_ratio(isParamValid("linear_iteration_ratio")
                                ? getParam<unsigned>("linear_iteration_ratio")
                                : 25), // Default to 25
    _adaptive_timestepping(false),
    _pps_value(isParamValid("postprocessor_dtlim") ? &getPostprocessorValue("postprocessor_dtlim")
                                                   : NULL),
    _timestep_limiting_function(NULL),
    _piecewise_timestep_limiting_function(NULL),
    _times(0),
    _max_function_change(-1),
    _force_step_every_function_point(getParam<bool>("force_step_every_function_point")),
    _tfunc_times(getParam<std::vector<Real>>("time_t").begin(),
                 getParam<std::vector<Real>>("time_t").end()),
    _time_ipol(getParam<std::vector<Real>>("time_t"), getParam<std::vector<Real>>("time_dt")),
    _use_time_ipol(_time_ipol.getSampleSize() > 0),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_factor(getParam<Real>("cutback_factor")),
    _nl_its(declareRestartableData<unsigned int>("nl_its", 0)),
    _l_its(declareRestartableData<unsigned int>("l_its", 0)),
    _cutback_occurred(declareRestartableData<bool>("cutback_occurred", false)),
    _at_function_point(false)
{
  if (isParamValid("optimal_iterations"))
  {
    _adaptive_timestepping = true;
    _optimal_iterations = getParam<int>("optimal_iterations");

    if (isParamValid("iteration_window"))
      _iteration_window = getParam<int>("iteration_window");
    else
      _iteration_window = ceil(_optimal_iterations / 5.0);
  }
  else
  {
    if (isParamValid("iteration_window"))
      mooseError("'optimal_iterations' must be used for 'iteration_window' to be used");
    if (isParamValid("linear_iteration_ratio"))
      mooseError("'optimal_iterations' must be used for 'linear_iteration_ratio' to be used");
  }

  if (isParamValid("timestep_limiting_function"))
    _max_function_change =
        isParamValid("max_function_change") ? getParam<Real>("max_function_change") : -1;
  else if (isParamValid("max_function_change"))
    mooseError("'timestep_limiting_function' must be used for 'max_function_change' to be used");
}

void
IterationAdaptiveDT::init()
{
  if (isParamValid("timestep_limiting_function"))
  {
    _timestep_limiting_function =
        &_fe_problem.getFunction(getParam<FunctionName>("timestep_limiting_function"),
                                 isParamValid("_tid") ? getParam<THREAD_ID>("_tid") : 0);
    _piecewise_timestep_limiting_function = dynamic_cast<Piecewise *>(_timestep_limiting_function);

    if (_piecewise_timestep_limiting_function)
    {
      unsigned int time_size = _piecewise_timestep_limiting_function->functionSize();
      _times.resize(time_size);

      for (unsigned int i = 0; i < time_size; ++i)
        _times[i] = _piecewise_timestep_limiting_function->domain(i);
    }
    else
      mooseError("timestep_limiting_function must be a Piecewise function");
  }
}

void
IterationAdaptiveDT::preExecute()
{
  TimeStepper::preExecute();

  // Delete all tfunc times that are at or before the begin time
  while (!_tfunc_times.empty() && _time + _timestep_tolerance >= *_tfunc_times.begin())
    _tfunc_times.erase(_tfunc_times.begin());
}

Real
IterationAdaptiveDT::computeInitialDT()
{
  return _input_dt;
}

Real
IterationAdaptiveDT::computeDT()
{
  Real dt = _dt_old;

  if (_cutback_occurred)
  {
    _cutback_occurred = false;
    if (_adaptive_timestepping)
    {
      // Don't allow it to grow this step, but shrink if needed
      bool allowToGrow = false;
      computeAdaptiveDT(dt, allowToGrow);
    }
  }
  else if (_tfunc_last_step)
  {
    _tfunc_last_step = false;
    _sync_last_step = false;
    dt = _time_ipol.sample(_time_old);

    if (_verbose)
    {
      _console << "Setting dt to value specified by dt function: " << std::setw(9) << dt << '\n';
    }
  }
  else if (_sync_last_step)
  {
    _sync_last_step = false;
    dt = _dt_old;

    if (_verbose)
    {
      _console << "Setting dt to value used before sync: " << std::setw(9) << dt << '\n';
    }
  }
  else if (_adaptive_timestepping)
    computeAdaptiveDT(dt);
  else if (_use_time_ipol)
    dt = computeInterpolationDT();
  else
  {
    dt *= _growth_factor;
    if (dt > _dt_old * _growth_factor)
      dt = _dt_old * _growth_factor;
  }

  return dt;
}

bool
IterationAdaptiveDT::constrainStep(Real & dt)
{
  bool at_sync_point = TimeStepper::constrainStep(dt);

  // Limit the timestep to postprocessor value

  limitDTToPostprocessorValue(dt);

  // Limit the timestep to limit change in the function
  limitDTByFunction(dt);

  // Adjust to the next tfunc time if needed
  if (!_tfunc_times.empty() && _time + dt + _timestep_tolerance >= *_tfunc_times.begin())
  {
    dt = *_tfunc_times.begin() - _time;

    if (_verbose)
    {
      _console << "Limiting dt to sync with dt function time: " << std::setw(9)
               << *_tfunc_times.begin() << " dt: " << std::setw(9) << dt << '\n';
    }
  }

  return at_sync_point;
}

Real
IterationAdaptiveDT::computeFailedDT()
{
  _cutback_occurred = true;

  // Can't cut back any more
  if (_dt <= _dt_min)
    mooseError("Solve failed and timestep already at dtmin, cannot continue!");

  if (_verbose)
  {
    _console << "\nSolve failed with dt: " << std::setw(9) << _dt
             << "\nRetrying with reduced dt: " << std::setw(9) << _dt * _cutback_factor << '\n';
  }
  else
    _console << "\nSolve failed, cutting timestep.\n";

  return _dt * _cutback_factor;
}

void
IterationAdaptiveDT::limitDTToPostprocessorValue(Real & limitedDT)
{
  if (_pps_value)
  {
    if (*_pps_value > _dt_min && limitedDT > *_pps_value)
    {
      limitedDT = *_pps_value;

      if (_verbose)
        _console << "Limiting dt to postprocessor value. dt = " << limitedDT << '\n';
    }
  }
}

void
IterationAdaptiveDT::limitDTByFunction(Real & limitedDT)
{
  Real orig_dt = limitedDT;

  if (_timestep_limiting_function)
  {
    Point dummyPoint;
    Real oldValue = _timestep_limiting_function->value(_time_old, dummyPoint);
    Real newValue = _timestep_limiting_function->value(_time_old + limitedDT, dummyPoint);
    Real change = std::abs(newValue - oldValue);

    if (_max_function_change > 0.0 && change > _max_function_change)
    {
      do
      {
        limitedDT /= 2.0;
        newValue = _timestep_limiting_function->value(_time_old + limitedDT, dummyPoint);
        change = std::abs(newValue - oldValue);
      } while (change > _max_function_change);
    }
  }

  _at_function_point = false;
  if (_piecewise_timestep_limiting_function && _force_step_every_function_point)
  {
    for (unsigned int i = 0; i + 1 < _times.size(); ++i)
    {
      if (_time >= _times[i] && _time < _times[i + 1])
      {
        if (limitedDT > _times[i + 1] - _time - _timestep_tolerance)
        {
          limitedDT = _times[i + 1] - _time;
          _at_function_point = true;
        }
        break;
      }
    }
  }

  if (_verbose && limitedDT != orig_dt)
  {
    if (_at_function_point)
      _console << "Limiting dt to match function point. dt = ";
    else
      _console << "Limiting dt to limit change in function. dt = ";

    _console << limitedDT << '\n';
  }
}

void
IterationAdaptiveDT::computeAdaptiveDT(Real & dt, bool allowToGrow, bool allowToShrink)
{
  const unsigned int growth_nl_its(
      _optimal_iterations > _iteration_window ? _optimal_iterations - _iteration_window : 0);
  const unsigned int shrink_nl_its(_optimal_iterations + _iteration_window);
  const unsigned int growth_l_its(_optimal_iterations > _iteration_window
                                      ? _linear_iteration_ratio *
                                            (_optimal_iterations - _iteration_window)
                                      : 0);
  const unsigned int shrink_l_its(_linear_iteration_ratio *
                                  (_optimal_iterations + _iteration_window));

  if (allowToGrow && (_nl_its < growth_nl_its && _l_its < growth_l_its))
  {
    // Grow the timestep
    dt *= _growth_factor;

    if (_verbose)
    {
      _console << "Growing dt: nl its = " << _nl_its << " < " << growth_nl_its
               << " && lin its = " << _l_its << " < " << growth_l_its << " old dt: " << std::setw(9)
               << _dt_old << " new dt: " << std::setw(9) << dt << '\n';
    }
  }
  else if (allowToShrink && (_nl_its > shrink_nl_its || _l_its > shrink_l_its))
  {
    // Shrink the timestep
    dt *= _cutback_factor;

    if (_verbose)
    {
      _console << "Shrinking dt: nl its = " << _nl_its << " > " << shrink_nl_its
               << " || lin its = " << _l_its << " > " << shrink_l_its << " old dt: " << std::setw(9)
               << _dt_old << " new dt: " << std::setw(9) << dt << '\n';
    }
  }
}

Real
IterationAdaptiveDT::computeInterpolationDT()
{
  Real dt = _time_ipol.sample(_time_old);

  if (dt > _dt_old * _growth_factor)
  {
    dt = _dt_old * _growth_factor;

    if (_verbose)
    {
      _console << "Growing dt to recover from cutback. "
               << " old dt: " << std::setw(9) << _dt_old << " new dt: " << std::setw(9) << dt
               << '\n';
    }
  }

  return dt;
}

void
IterationAdaptiveDT::rejectStep()
{
  TimeStepper::rejectStep();
}

void
IterationAdaptiveDT::acceptStep()
{
  TimeStepper::acceptStep();

  while (!_tfunc_times.empty() && _time + _timestep_tolerance >= *_tfunc_times.begin())
  {
    if (std::abs(_time - *_tfunc_times.begin()) <= _timestep_tolerance)
      _tfunc_last_step = true;

    _tfunc_times.erase(_tfunc_times.begin());
  }

  _nl_its = _fe_problem.getNonlinearSystemBase().nNonlinearIterations();
  _l_its = _fe_problem.getNonlinearSystemBase().nLinearIterations();

  if ((_at_function_point || _executioner.atSyncPoint()) &&
      _dt + _timestep_tolerance < _executioner.unconstrainedDT())
  {
    _dt_old = _fe_problem.dtOld();
    _sync_last_step = true;

    if (_verbose)
    {
      _console << "Sync point hit in current step, using previous dt for old dt: " << std::setw(9)
               << _dt_old << '\n';
    }
  }
  else
    _dt_old = _dt;
}
