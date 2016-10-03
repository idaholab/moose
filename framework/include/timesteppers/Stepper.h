#pragma once

#include <functional>

#include "LinearInterpolation.h"
#include "libmesh/numeric_vector.h"

struct StepperInfo {
  int call_count;

  // time info
  double prev_dt;
  unsigned int time;

  // solve stats
  unsigned int nonlin_iters;
  unsigned int lin_iters;
  bool converged;

  // solution stuff
  NumericVector<Number>* soln_nonlin;
  NumericVector<Number>* aux_soln;
  NumericVector<Number>* predicted_soln;

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

class EveryNStepper : public Stepper
{
public:
  EveryNStepper(Stepper* s, int every_n) : _stepper(s), _n(every_n) { }

  virtual double advance(const StepperInfo* si) {
    if (si->call_count % _n == 0)
      return _stepper->advance(si);
    else
      return si->prev_dt;
  }

private:
  Stepper* _stepper;
  int _n;
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

    unsigned int shrink_nl_its = _optimal_iters + _iter_window;
    unsigned int shrink_l_its = _lin_iter_ratio*(_optimal_iters + _iter_window);
    unsigned int growth_nl_its = 0;
    unsigned int growth_l_its = 0;
    if (_optimal_iters > _iter_window)
    {
       growth_nl_its = _optimal_iters - _iter_window;
       growth_l_its = _lin_iter_ratio * (_optimal_iters - _iter_window);
    }

    if (can_grow && (si->nonlin_iters < growth_nl_its && si->lin_iters < growth_l_its))
      return si->prev_dt * _growth_factor;
    else if (can_shrink && (si->nonlin_iters > shrink_nl_its || si->lin_iters > shrink_l_its))
      return si->prev_dt * _shrink_factor;
    else
      return si->prev_dt;
  };

  double _growth_factor;
  double _shrink_factor;
  unsigned int _optimal_iters;
  unsigned int _iter_window;
  unsigned int _lin_iter_ratio;
};

