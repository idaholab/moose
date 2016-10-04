#pragma once

#include <functional>

#include "MooseError.h"
#include "LinearInterpolation.h"
#include "libmesh/numeric_vector.h"

struct StepperInfo {
  // The number of times the simulation has requested a dt.
  // This is equal to one on the first call fo advance(...).
  int step_count;

  // time info
  double time;
  double prev_dt;
  double prev_prev_dt;
  std::string time_integrator;

  // solve stats
  unsigned int nonlin_iters;
  unsigned int lin_iters;
  bool converged;
  double solve_time_secs;

  // solution stuff
  NumericVector<Number>* soln_nonlin;
  NumericVector<Number>* soln_aux;
  NumericVector<Number>* soln_predicted;

  // backup/restore
  bool sched_backup;
  bool sched_restore;
  double restore_time;
};

class Stepper
{
public:
  // Implementations of advance should strive to be idempotent.  Return the
  // next value to use as dt.
  virtual double advance(const StepperInfo* si) = 0;
};

/////////////////////////////////////////////////////////////
///////////////// Stepper Implementations ///////////////////
/////////////////////////////////////////////////////////////

// Always returns a single, unchanging, user-specified dt.
class ConstStepper : public Stepper
{
public:
  ConstStepper(double dt) : _dt(dt) { }

  virtual double advance(const StepperInfo* si) { return _dt; }

private:
  double _dt;
};

// Calls an underlying stepper to compute dt. Modifies dt to be within
// specified fixed lower and upper bounds (or throws an error - depending on
// configuration).
class BoundsStepper : public Stepper
{
public:
  // If throw_err is true, an exception is thrown if the underlying stepper's
  // advance function returns an out of bounds dt.  Otherwise, out-of-bounds
  // cases result in dt being set to the corresponding min or max bound.
  BoundsStepper(Stepper* s, double dt_min, double dt_max, bool throw_err) : _stepper(s), _min(dt_min), _max(dt_max), _err(throw_err) { }

  virtual double advance(const StepperInfo* si) {
    double dt = _stepper->advance(si);
    if (_err && (dt < _min || dt > _max))
      throw "time step is out of bounds";
    else if (dt < _min)
      return _min;
    else if (dt > _max)
      return _max;
    return dt;
  }

private:
  Stepper* _stepper;
  double _min;
  double _max;
  bool _err;
};

// Calculates dt in order to hit specified fixed times. This is robust if the
// dt used to evolve the actual simulation ended up being changed from this
// dt.  Returns previous dt after all fixed times have been passed. Never
// returns a negative dt
class FixedPointStepper : public Stepper
{
public:
  FixedPointStepper(std::vector<double> times, double tol) : _times(times), _time_tol(tol) { }

  virtual double advance(const StepperInfo* si) {
    if (_times.size() == 0)
      return si->prev_dt;

    for (int i = 0; i < _times.size(); i++)
    {
      double t0 = _times[i];
      if (t0 - _time_tol > si->time)
        return t0 - si->time;
    }
    return si->prev_dt;
  }

private:
  std::vector<double> _times;
  double _time_tol;
};

// Delegates dt calculations to an underlying stepper.  If that stepper's
// calculated new dt violates the equation "dt / prev_dt <= max_ratio", then
// this stepper returns dt modified to be equal to "prev_dt * max_ratio"
class MaxRatioStepper : public Stepper
{
public:
  MaxRatioStepper(Stepper* s, double max_ratio) : _stepper(s), _max_ratio(max_ratio) { }

  virtual double advance(const StepperInfo* si)
  {
    double dt = _stepper->advance(si);
    if (si->prev_dt > 0 && dt / si->prev_dt > _max_ratio)
      dt = si->prev_dt * _max_ratio;
    return dt;
  }

private:
  Stepper* _stepper;
  double _max_ratio;
};

// Delegates dt calculations to an underlying stepper only every N steps.
// Otherwise, it returns the previous dt.
class EveryNStepper : public Stepper
{
public:
  EveryNStepper(Stepper* s, int every_n) : _stepper(s), _n(every_n) { }

  virtual double advance(const StepperInfo* si) {
    if (si->step_count % _n == 1)
      return _stepper->advance(si);
    else
      return si->prev_dt;
  }

private:
  Stepper* _stepper;
  int _n;
};

// Uses an underlying stepper to compute dt.  If the actuall simulation-used
// previous dt was not what the underlying stepper returned the prior call to
// advance, this stepper returns/retries that dt value.
class RetryUnusedStepper : public Stepper
{
public:
  RetryUnusedStepper(Stepper* s) : _stepper(s), _prev_dt(0) { }

  virtual double advance(const StepperInfo* si) {
    if (_prev_dt != 0 && si->prev_dt != _prev_dt)
      return _prev_dt;

    _prev_dt  = _stepper->advance(si);
    return _prev_dt;
  }

private:
  Stepper* _stepper;
  double _prev_dt;
};

// ConstrFuncStepper reduces the returned dt of an underlying stepper by
// factors of two until the difference between a given limiting function
// evaluated at t_curr and t_next is less than a specified maximum difference.
class ConstrFuncStepper : public Stepper {
public:
  ConstrFuncStepper(Stepper* s, std::function<double(double)> func, double max_diff)
    : _stepper(s),
      _func(func),
      _max_diff(max_diff)
  { }

  virtual double advance(const StepperInfo* si) {
    double dt = _stepper->advance(si);
    double f_curr = _func(si->time);
    double df = std::abs(_func(si->time + dt) - f_curr);
    while(df > _max_diff)
    {
      dt /= 2.0;
      df = std::abs(_func(si->time + dt) - f_curr);
    }
    return dt;
  }

private:
  Stepper* _stepper;
  std::function<double (double t)> _func;
  double _max_diff;
};

class PiecewiseStepper : public Stepper
{
public:
  PiecewiseStepper(std::vector<double> times, std::vector<double> dts) : _lin(times, dts) { }

  virtual double advance(const StepperInfo* si) {
    return _lin.sample(si->time);
  }

private:
  LinearInterpolation _lin;
};

class MinOfStepper : public Stepper
{
public:
  MinOfStepper(Stepper* a, Stepper* b) : _a(a), _b(b) { }

  virtual double advance(const StepperInfo* si)
  {
    return std::min(_a->advance(si), _b->advance(si));
  }

private:
  Stepper* _a;
  Stepper* _b;
};

class AdaptiveStepper : public Stepper
{
public:
  virtual double advance(const StepperInfo* si) {
    bool can_shrink = true;
    bool can_grow = si->converged;

    unsigned int shrink_nl_its = optimal_iters + iter_window;
    unsigned int shrink_l_its = lin_iter_ratio*(optimal_iters + iter_window);
    unsigned int growth_nl_its = 0;
    unsigned int growth_l_its = 0;
    if (optimal_iters > iter_window)
    {
       growth_nl_its = optimal_iters - iter_window;
       growth_l_its = lin_iter_ratio * (optimal_iters - iter_window);
    }

    if (can_grow && (si->nonlin_iters < growth_nl_its && si->lin_iters < growth_l_its))
      return si->prev_dt * growth_factor;
    else if (can_shrink && (si->nonlin_iters > shrink_nl_its || si->lin_iters > shrink_l_its))
      return si->prev_dt * shrink_factor;
    else
      return si->prev_dt;
  };

  double growth_factor;
  double shrink_factor;
  unsigned int optimal_iters;
  unsigned int iter_window;
  unsigned int lin_iter_ratio;
};

// Original PredictorCorrector timestepper behavior can be achieved by
// wrapping steppers like so: EveryNStepper(MaxRatioStepper(PredictorCorrector()))
class PredictorCorrector : public Stepper
{
public:
  PredictorCorrector(int start_adapting, double e_tol, double scaling_param) :
      _start_adapting(start_adapting),
      _e_tol(e_tol),
      _scale_param(scaling_param)
  { }

  virtual double advance(const StepperInfo* si)
  {
    if (!si->converged)
      return si->prev_dt;
    // original Predictor stepper actually used the Real valued time step
    // instead of step_count which is used here - which doesn't make sense for
    // this check since if t_0 != 0 or dt != 1 would make it so the predictor
    // might not have a prediction by the time _start_adapting was called -
    // this could be changed back like the original, but then, start_adapting
    // needs to be carefully documented.
    if (si->step_count < _start_adapting)
      return si->prev_dt;
    if (si->soln_nonlin == nullptr || si->soln_aux == nullptr || si->soln_predicted == nullptr)
      throw "no predicted solution available";

    double error = estimateTimeError(si);
    double infnorm = si->soln_nonlin->linfty_norm();
    double e_max = 1.1 * _e_tol * infnorm;
    // TODO: console handling? ==> _console << "Time Error Estimate: " << _error << std::endl;

    double dt = si->prev_dt * _scale_param * std::pow(infnorm * _e_tol / error, 1.0 / 3.0);
    return dt;
  }

private:
  double estimateTimeError(const StepperInfo* si)
  {
    NumericVector<Number>& soln = *si->soln_nonlin;
    NumericVector<Number>& predicted = *si->soln_predicted;

    std::string scheme = si->time_integrator;
    double dtprev = si->prev_prev_dt;
    double dt = si->prev_dt;
    if (scheme == "CrankNicolson")
    {
      predicted -= soln;
      predicted *= (dt) / (3.0 * (dt + dtprev));
      return predicted.l2_norm();
    }
    else if (scheme == "BDF2")
    {
      predicted *= -1.0;
      predicted += soln;
      double topcalc = 2.0 * (dt + dtprev) * (dt + dtprev);
      double bottomcalc = 6.0 * dt * dt + 12.0 * dt * dtprev + 5.0 * dtprev * dtprev;
      predicted *= topcalc / bottomcalc;
      return predicted.l2_norm();
    }

    return -1;
  }

  int _start_adapting;
  double _e_tol;
  double _scale_param;
};

class DT2 : public Stepper
{
public:
  DT2() { }

  virtual double advance(const StepperInfo* si) {
    throw "unimplemented";
  }

private:
  int foo;
};

