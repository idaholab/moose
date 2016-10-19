#pragma once

#define MAKETOGETHER(a,b) a##b
#define DBG MAKETOGETHER(std::c,out)

#include <functional>

#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "libmesh/numeric_vector.h"

class Logger
{
public:
  Logger(std::string loc) : _loc(loc), _dt(-1)
  { if (on)
    {
      DBG << "[LOG] " << std::string(level*4, ' ') << "- entering " << _loc << "\n";
      level++;
    }
  }
  ~Logger()
  {
    if (on)
    {
      level--;
      DBG << "[LOG] " << std::string(level*4, ' ') << "- leaving " << _loc << " (dt=" << _dt << ")\n";
    }
  }
  double val(double dt)
  {
    _dt = dt;
    return dt;
  }
  static int level;
  static bool on;
  std::string _loc;
  double _dt;
};

struct StepperInfo {
  // The number of times the simulation has requested a dt.
  // This is equal to one on the first call fo advance(...).
  int step_count;

  // time info
  double time;
  double prev_dt;
  double prev_prev_dt;
  double prev_prev_prev_dt;
  std::string time_integrator;

  // solve stats
  unsigned int nonlin_iters;
  unsigned int lin_iters;
  bool converged;
  bool prev_converged;
  double prev_solve_time_secs;
  double prev_prev_solve_time_secs;
  double prev_prev_prev_solve_time_secs;

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
  typedef std::unique_ptr<Stepper> Ptr;

  virtual ~Stepper() = default;
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
  ConstStepper(double dt) : _dt(dt)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("Const");
    return l.val(_dt);
  }

private:
  double _dt;
};

// Calls an underlying stepper to compute dt. Modifies dt to be within
// specified fixed lower and upper bounds (or throws an error - depending on
// configuration).
class DTLimitStepper : public Stepper
{
public:
  // If throw_err is true, an exception is thrown if the underlying stepper's
  // advance function returns an out of bounds dt.  Otherwise, out-of-bounds
  // cases result in dt being set to the corresponding min or max bound.
  DTLimitStepper(Stepper* s, double dt_min, double dt_max, bool throw_err) : _stepper(s), _min(dt_min), _max(dt_max), _err(throw_err)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("DTLimit");
    double dt = _stepper->advance(si);
    if (_err && (dt < _min || dt > _max))
      throw "time step is out of bounds";
    else if (dt < _min)
      return l.val(_min);
    else if (dt > _max)
      return l.val(_max);
    return l.val(dt);
  }

private:
  Ptr _stepper;
  double _min;
  double _max;
  bool _err;
};

// Calls an underlying stepper to compute dt. Modifies dt to maintain the
// simulation time within specified fixed lower and upper bounds (or throws an
// error - depending on configuration).
class BoundsStepper : public Stepper
{
public:
  // If throw_err is true, an exception is thrown if the underlying stepper's
  // advance function returns a dt that causes an out of bounds simulation
  // time.  Otherwise, out-of-bounds cases result in dt being set to hit the
  // corresponding time bound.
  BoundsStepper(Stepper* s, double t_min, double t_max, bool throw_err) : _stepper(s), _min(t_min), _max(t_max), _err(throw_err)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("Bounds");
    double dt = _stepper->advance(si);
    double t = si->time + dt;
    if (_err && (t < _min || t > _max))
      throw "time step is out of bounds";
    else if (t < _min)
      return l.val(_min - si->time);
    else if (t > _max)
      return l.val(_max - si->time);
    return l.val(dt);
  }

private:
  Ptr _stepper;
  double _min;
  double _max;
  bool _err;
};

// Calculates dt in order to hit specified fixed times. This is robust if the
// dt used to evolve the actual simulation ended up being changed from this
// dt.  Returns previous dt after all fixed times have been passed. Never
// returns a negative dt
//
// Consider making this stepper return 1e100 or prev_dt after sim time is greater than
// all specified times in sequence.  1e100 would help it work more correctly with
// MinOfStepper and BoundsSteper, etc.  prev_dt would make sense possibly in
// other contexts.
class FixedPointStepper : public Stepper
{
//#define FIXED_STEPPER_RETURN si->prev_dt
#define FIXED_STEPPER_RETURN 1e100
public:
  FixedPointStepper(std::vector<double> times, double tol) : _times(times), _time_tol(tol)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("FixedPoint");
    if (_times.size() == 0)
      return l.val(FIXED_STEPPER_RETURN);

    for (int i = 0; i < _times.size(); i++)
    {
      double t0 = _times[i];
      if (si->time < t0 - _time_tol)
        return l.val(t0 - si->time);
    }
    return l.val(FIXED_STEPPER_RETURN);
  }

private:
  std::vector<double> _times;
  double _time_tol;
};

// Alternating stepper calls one stepper between the specified simulation
// times and another between them.
class AlternatingStepper : public Stepper
{
public:
  AlternatingStepper(Stepper* on_steps, Stepper* between_steps, std::vector<double> times, double tol) :
      _on_steps(on_steps),
      _between_steps(between_steps),
      _times(times),
      _time_tol(tol)
  { }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("Alternating");
    for (int i = 0; i < _times.size(); i++)
    {
      if (std::abs(si->time - _times[i]) < _time_tol)
        return l.val(_on_steps->advance(si));
    }
    return l.val(_between_steps->advance(si));
  }

private:
  Ptr _on_steps;
  Ptr _between_steps;
  std::vector<double> _times;
  double _time_tol;
};

// Delegates dt calculations to an underlying stepper.  If that stepper's
// calculated new dt violates the equation "dt / prev_dt <= max_ratio", then
// this stepper returns dt modified to be equal to "prev_dt * max_ratio"
class MaxRatioStepper : public Stepper
{
public:
  MaxRatioStepper(Stepper* s, double max_ratio) : _stepper(s), _max_ratio(max_ratio)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("MaxRatio");
    double dt = _stepper->advance(si);
    if (si->prev_dt > 0 && dt / si->prev_dt > _max_ratio)
      dt = si->prev_dt * _max_ratio;
    return l.val(dt);
  }

private:
  Ptr _stepper;
  double _max_ratio;
};

// Delegates dt calculations to an underlying stepper only every N steps (not every n calls to advance).
// Otherwise, it returns the previous dt.  If an offset is provided, the first call to advance will occur on the offset'th step.
class EveryNStepper : public Stepper
{
public:
  // offset must be smaller than every_n.
  EveryNStepper(Stepper* s, int every_n, int offset = 0) : _stepper(s), _n(every_n), _offset(offset)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("EveryN");
    if ((si->step_count + (_n - _offset) - 1) % _n == 0)
      return l.val(_stepper->advance(si));
    else
      return l.val(si->prev_dt);
  }

private:
  Ptr _stepper;
  int _n;
  int _offset;
};

class ReturnPtrStepper : public Stepper
{
public:
  ReturnPtrStepper(const double * dt_store) : _dt_store(dt_store)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("ReturnPtr");
    return l.val(*_dt_store);
  }

private:
  const double * _dt_store;
};

// Returns the dt of the underlying stepper unmodified.  Stores/remembers this
// dt which can be viewed/connected to via the pointer returned by dtPtr().
// The underlying stepper must be set via setStepper - this allows
// InstrumentedStepper to be created before other steppers which may want to
// use the dt_store pointer.
class InstrumentedStepper : public Stepper
{
public:
  InstrumentedStepper() : _stepper(nullptr), _dt_store(0)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("Instrumented");
    if (!_stepper)
      throw "InstrumentedStepper's inner stepper not set";

    _dt_store = _stepper->advance(si);
    return l.val(_dt_store);
  }

  void setStepper(Stepper* s)
  {
    _stepper.reset(s);
  }

  double * dtPtr()
  {
    return &_dt_store;
  }

private:
  Ptr _stepper;
  double _dt_store;
};

// Uses an underlying stepper to compute dt.  If the actuall simulation-used
// previous dt was not what the underlying stepper returned the prior call to
// advance, this stepper returns/retries that dt value.
class RetryUnusedStepper : public Stepper
{
public:
  RetryUnusedStepper(Stepper* s, double tol, bool prev_prev) : _stepper(s), _tol(tol), _prev_prev(prev_prev), _prev_dt(0)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("RetryUnused");
    if (_prev_dt != 0 && std::abs(si->prev_dt - _prev_dt) > _tol)
    {
      if (_prev_prev)
      {
        _prev_dt = si->prev_prev_dt;
        return l.val(si->prev_prev_dt);
      }
      else
        return l.val(_prev_dt);
    }

    _prev_dt  = _stepper->advance(si);
    return l.val(_prev_dt);
  }

private:
  Ptr _stepper;
  double _tol;
  bool _prev_prev;
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

  virtual double advance(const StepperInfo* si)
  {
    Logger l("ConstrFunc");
    double dt = _stepper->advance(si);
    double f_curr = _func(si->time);
    double df = std::abs(_func(si->time + dt) - f_curr);
    while (_max_diff > 0 && df > _max_diff)
    {
      dt /= 2.0;
      df = std::abs(_func(si->time + dt) - f_curr);
    }
    return l.val(dt);
  }

private:
  Ptr _stepper;
  std::function<double (double t)> _func;
  double _max_diff;
};



class PiecewiseStepper : public Stepper
{
public:
  PiecewiseStepper(std::vector<double> times, std::vector<double> dts, bool interpolate = true) :
      _times(times),
      _dts(dts),
      _interp(interpolate),
      _lin(times, dts)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("Piecewise");
    if (_interp)
      return l.val(_lin.sample(si->time));

    if (MooseUtils::relativeFuzzyGreaterEqual(si->time, _times.back()))
      return l.val(_times.back());

    for (int i = 0; i < _times.size() - 1; i++)
      if (MooseUtils::relativeFuzzyLessThan(si->time, _times[i + 1]))
        return l.val(_dts[i]);
    return l.val(_dts.back());
  }

private:
  std::vector<double> _times;
  std::vector<double> _dts;
  bool _interp;
  LinearInterpolation _lin;
};

// MinOfStepper returns the smaller of two dt's from two underlying steppers.
// It returns the preferred stepper's dt if:
//
//     "dt_preferred - tolerance < dt_alternate"
//
// Otherwise it returns the alternate stepper's dt.
class MinOfStepper : public Stepper
{
public:
  // stepper a is preferred
  MinOfStepper(Stepper* a, Stepper* b, double tol) : _a(a), _b(b), _tol(tol)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("MinOf");
    double dta = _a->advance(si);
    double dtb = _b->advance(si);
    if (dta - _tol < dtb)
      return l.val(dta);
    return l.val(dtb);
  }

private:
  Ptr _a;
  Ptr _b;
  double _tol;
};

class AdaptiveStepper : public Stepper
{
public:
  AdaptiveStepper(
        unsigned int optimal_iters,
        unsigned int iter_window,
        double lin_iter_ratio,
        double shrink_factor,
        double growth_factor
      ) :
      _optimal_iters(optimal_iters),
      _iter_window(iter_window),
      _lin_iter_ratio(lin_iter_ratio),
      _shrink_factor(shrink_factor),
      _growth_factor(growth_factor)
      { }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("Adaptive");
    bool can_shrink = true;
    bool can_grow = si->converged && si->prev_converged;

    unsigned int growth_nl_its = 0;
    unsigned int growth_l_its = 0;
    unsigned int shrink_nl_its = _optimal_iters + _iter_window;
    unsigned int shrink_l_its = _lin_iter_ratio*(_optimal_iters + _iter_window);
    if (_optimal_iters > _iter_window)
    {
       growth_nl_its = _optimal_iters - _iter_window;
       growth_l_its = _lin_iter_ratio * (_optimal_iters - _iter_window);
    }

    if (can_grow && (si->nonlin_iters < growth_nl_its && si->lin_iters < growth_l_its))
      return l.val(si->prev_dt * _growth_factor);
    else if (can_shrink && (!si->converged || si->nonlin_iters > shrink_nl_its || si->lin_iters > shrink_l_its))
      return l.val(si->prev_dt * _shrink_factor);
    else
      return l.val(si->prev_dt);
  };

private:
  unsigned int _optimal_iters;
  unsigned int _iter_window;
  double _lin_iter_ratio;
  double _shrink_factor;
  double _growth_factor;
};

class SolveTimeAdaptiveStepper : public Stepper
{
public:
  SolveTimeAdaptiveStepper(int initial_direc, double percent_change) :
      _percent_change(percent_change),
      _direc(initial_direc),
      _n_steps(0)
      { }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("SolveTimeAdaptive");
    double ratio = si->prev_solve_time_secs / si->prev_dt;
    double prev_ratio = si->prev_prev_solve_time_secs / si->prev_prev_dt;
    double prev_prev_ratio = si->prev_prev_prev_solve_time_secs / si->prev_prev_prev_dt;

    _n_steps++;
    // this is this way in order to mirror original SolutionTimeAdaptiveDT
    // stepper behavior.  However, it might be better to compare prev_ratio to
    // prev_prev ratio instead.
    if (ratio > prev_ratio && ratio > prev_prev_ratio && _n_steps > 1)
    {
      _direc *= -1;
      _n_steps = 0;
    }

    return l.val(si->prev_dt + si->prev_dt * _percent_change * _direc);
  }
private:
  double _percent_change;
  int _direc;
  int _n_steps;
};

class StartupStepper : public Stepper
{
public:
  StartupStepper(Stepper* s, double initial_dt, int n_steps = 1) : _stepper(s), _dt(initial_dt), _n(n_steps)
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("Startup");
    if (si->step_count <= _n)
      return l.val(_dt);
    return l.val(_stepper->advance(si));
  }

private:
  Ptr _stepper;
  double _dt;
  int _n;
};

class IfConvergedStepper : public Stepper
{
public:
  IfConvergedStepper(Stepper* if_converged, Stepper* if_not_converged) :
      _converged(if_converged),
      _not_converged(if_not_converged)
      { }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("IfConverged");
    if (si->converged)
      return l.val(_converged->advance(si));
    else
      return l.val(_not_converged->advance(si));
  }

private:
  Ptr _converged;
  Ptr _not_converged;
};


class GrowShrinkStepper : public Stepper
{
public:
  // if source_dt != nullptr, the dt returned by it is used to grow or shrink
  // on accordingly
  GrowShrinkStepper(double shrink_factor, double growth_factor, Stepper* source_dt = nullptr) :
      _grow_fac(growth_factor),
      _shrink_fac(shrink_factor),
      _source_dt(source_dt)
      { }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("GrowShrink");
    double dt_init = si->prev_dt;
    if (_source_dt)
      dt_init = _source_dt->advance(si);

    if (!si->converged)
      return l.val(dt_init * _shrink_fac);
    else
      return l.val(dt_init * _grow_fac);
  }

private:
  double _grow_fac;
  double _shrink_fac;
  Ptr _source_dt;
};

class PredictorCorrectorStepper : public Stepper
{
public:
  PredictorCorrectorStepper(int start_adapting, double e_tol, double scaling_param) :
      _start_adapting(start_adapting),
      _e_tol(e_tol),
      _scale_param(scaling_param)
  { }

  virtual double advance(const StepperInfo* si)
  {
    Logger l("PredictorCorrector");
    if (!si->converged)
      return l.val(si->prev_dt);
    // original Predictor stepper actually used the Real valued time step
    // instead of step_count which is used here - which doesn't make sense for
    // this check since if t_0 != 0 or dt != 1 would make it so the predictor
    // might not have a prediction by the time _start_adapting was called -
    // this could be changed back like the original, but then, start_adapting
    // needs to be carefully documented.
    if (si->step_count < _start_adapting)
      return l.val(si->prev_dt);
    if (si->soln_nonlin == nullptr || si->soln_aux == nullptr || si->soln_predicted == nullptr)
      throw "no predicted solution available";

    double error = estimateTimeError(si);
    double infnorm = si->soln_nonlin->linfty_norm();
    double e_max = 1.1 * _e_tol * infnorm;

    if (error > e_max)
      return  l.val(si->prev_dt * 0.5);
    return l.val(si->prev_dt * _scale_param * std::pow(infnorm * _e_tol / error, 1.0 / 3.0));
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

    throw "unsupported time integration scheme";
  }

  int _start_adapting;
  double _e_tol;
  double _scale_param;
};

class DT2Stepper : public Stepper
{
public:
  DT2Stepper()
  {
  }

  virtual double advance(const StepperInfo* si)
  {
    throw "unimplemented";
  }

private:
  int foo;
};

