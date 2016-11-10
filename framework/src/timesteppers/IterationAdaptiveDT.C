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
#include "Stepper.h"

template<>
InputParameters validParams<IterationAdaptiveDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addClassDescription("Adjust the timestep based on the number of iterations");
  params.addParam<int>("optimal_iterations", "The target number of nonlinear iterations for adaptive timestepping");
  params.addParam<int>("iteration_window", "Attempt to grow/shrink timestep if the iteration count is below/above 'optimal_iterations plus/minus iteration_window' (default = optimal_iterations/5).");
  params.addParam<unsigned>("linear_iteration_ratio", "The ratio of linear to nonlinear iterations to determine target linear iterations and window for adaptive timestepping (default = 25)");
  params.addParam<PostprocessorName>("postprocessor_dtlim", "If specified, the postprocessor value is used as an upper limit for the current time step length");
  params.addParam<FunctionName>("timestep_limiting_function", "A 'Piecewise' type function used to control the timestep by limiting the change in the function over a timestep");
  params.addParam<Real>("max_function_change", "The absolute value of the maximum change in timestep_limiting_function over a timestep");
  params.addParam<bool>("force_step_every_function_point", false, "Forces the timestepper to take a step that is consistent with points defined in the function");
  params.addRequiredParam<Real>("dt", "The default timestep size between solves");
  params.addParam<std::vector<Real> >("time_t", "The values of t");
  params.addParam<std::vector<Real> >("time_dt", "The values of dt");
  params.addParam<Real>("growth_factor", 2.0, "Factor to apply to timestep if easy convergence (if 'optimal_iterations' is specified) or if recovering from failed solve");
  params.addParam<Real>("cutback_factor", 0.5, "Factor to apply to timestep if difficult convergence (if 'optimal_iterations' is specified) or if solution failed");
  return params;
}

IterationAdaptiveDT::IterationAdaptiveDT(const InputParameters & parameters) :
    TimeStepper(parameters),
    PostprocessorInterface(this),
    _input_dt(getParam<Real>("dt")),
    _linear_iteration_ratio(isParamValid("linear_iteration_ratio") ? getParam<unsigned>("linear_iteration_ratio") : 25),  // Default to 25
    _adaptive_timestepping(false),
    _pps_value(isParamValid("postprocessor_dtlim") ? &getPostprocessorValue("postprocessor_dtlim") : nullptr),
    _timestep_limiting_function(nullptr),
    _piecewise_timestep_limiting_function(nullptr),
    _times(0),
    _max_function_change(-1),
    _force_step_every_function_point(getParam<bool>("force_step_every_function_point")),
    _tfunc_times(getParam<std::vector<Real> >("time_t").begin(), getParam<std::vector<Real> >("time_t").end()),
    _time_ipol(getParam<std::vector<Real> >("time_t"),
               getParam<std::vector<Real> >("time_dt")),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_factor(getParam<Real>("cutback_factor")),
    _tfunc_dts(getParam<std::vector<Real> >("time_dt"))
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
    _max_function_change = isParamValid("max_function_change") ?
                           getParam<Real>("max_function_change") : -1;
  else if (isParamValid("max_function_change"))
    mooseError("'timestep_limiting_function' must be used for 'max_function_change' to be used");
}

void
IterationAdaptiveDT::init()
{
  if (isParamValid("timestep_limiting_function"))
  {
    _timestep_limiting_function = &_fe_problem.getFunction(getParam<FunctionName>("timestep_limiting_function"), isParamValid("_tid") ? getParam<THREAD_ID>("_tid") : 0);
    _piecewise_timestep_limiting_function = dynamic_cast<Piecewise*>(_timestep_limiting_function);

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

StepperBlock *
IterationAdaptiveDT::buildStepper()
{
  Real tol = _executioner.timestepTol();
  int n_startup_steps = _executioner.n_startup_steps();
  Real end_time = _executioner.endTime();
  Real start_time = _executioner.getStartTime();
  Real dtmin = _executioner.dtMin();
  Real dtmax = _executioner.dtMax();
  bool half_transient = _app.halfTransient();
  bool use_time_ipol = _time_ipol.getSampleSize() > 0;

  StepperBlock * stepper = nullptr;

  std::vector<Real> time_list;
  std::vector<Real> dt_list;
  // this needs to occur before preExecute() is called on old-timestepper because that method modifies the original tfunc_times
  for (auto val : _tfunc_times)
    time_list.push_back(val);
  for (auto val : _tfunc_dts)
    dt_list.push_back(val);

  Function* t_limit_func = _timestep_limiting_function;
  std::vector<Real> piecewise_list = _times;

  if (_adaptive_timestepping)
  {
    stepper = new AdaptiveBlock(
          _optimal_iterations,
          _iteration_window,
          _linear_iteration_ratio,
          _cutback_factor, // shrink_factor
          _growth_factor
        );
    if (use_time_ipol)
      stepper = BaseStepper::between(new PiecewiseBlock(time_list, dt_list), stepper, time_list, tol);
  }
  else if (use_time_ipol)
      stepper = new PiecewiseBlock(time_list, dt_list);
  else
    // this should cover the final else clause in IterationAdaptiveDT::computeDT combined with
    // the no growth if _cutback_occurred - from the first if clause body.
    stepper = BaseStepper::converged(BaseStepper::mult(_growth_factor), BaseStepper::prevdt(), true);

  stepper = BaseStepper::converged(stepper, BaseStepper::mult(0.5));

  stepper = BaseStepper::initialN(BaseStepper::constant(_input_dt), stepper, std::max(1, n_startup_steps));
  // Original IterationAdaptiveDT stepper constrains to simulation end time
  // *before* applying other constraints - sometimes resulting in an
  // over-constrained dt - for example a dt divide-by-two algo to satisfy
  // function mapping constraints will start with a smaller dt than it would
  // have otherwise - and it might end up generating a dt that satisfies
  // simulation time end constraint without having to enforce that constraint
  // explicitly.  This behavior is undesirable.  The simulation end constraint should
  // be the last constraint enforced.
  if (!half_transient)
    stepper = BaseStepper::bounds(stepper, start_time, end_time); // This is stupid.
  if (t_limit_func)
  {
    stepper = new ConstrFuncBlock(
          stepper,
          // must capture pointer by value - not ref because scope
          [=](Real x)->Real{return t_limit_func->value(x, Point());},
          _max_function_change
        );
  }
  // Original IterationAdaptiveDT stepper does not retry prior dt if last dt
  // was constrained by dt min/max - whatever - so we need this stepper to be
  // inside the Retry stepper to reproduce that behavior
  stepper = BaseStepper::dtLimit(stepper, dtmin, dtmax);
  // this needs to go before RetryUnused
  if (_pps_value)
  {
    StepperBlock * s = BaseStepper::ptr(_pps_value);
    // startup stepper needed for initial case where pps_value hasn't been set
    // to anything yet.
    s = BaseStepper::initialN(BaseStepper::constant(1e100), s, n_startup_steps);
    stepper = BaseStepper::min(stepper, s);
  }

  if (time_list.size() > 0)
    stepper = BaseStepper::min(BaseStepper::fixedTimes(time_list, tol), stepper, tol);
  stepper = new RetryUnusedBlock(stepper, tol, true);
  if (_force_step_every_function_point && piecewise_list.size() > 0)
    stepper = BaseStepper::min(BaseStepper::fixedTimes(piecewise_list, tol), stepper, tol);

  return stepper;
}

