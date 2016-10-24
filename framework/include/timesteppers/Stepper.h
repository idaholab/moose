#pragma once

#include <functional>

#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "libmesh/numeric_vector.h"

#define MAKETOGETHER(a,b) a##b
#define DBG MAKETOGETHER(std::c,out)
#define STEPPER_LOGGING true

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
  std::unique_ptr<NumericVector<Number> > soln_nonlin;
  std::unique_ptr<NumericVector<Number> > soln_aux;
  std::unique_ptr<NumericVector<Number> > soln_predicted;
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
  ConstStepper(double dt);
  virtual double advance(const StepperInfo* si);
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
  DTLimitStepper(Stepper* s, double dt_min, double dt_max, bool throw_err);
  virtual double advance(const StepperInfo* si);
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
  BoundsStepper(Stepper* s, double t_min, double t_max, bool throw_err);
  virtual double advance(const StepperInfo* si);
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
  FixedPointStepper(std::vector<double> times, double tol);
  virtual double advance(const StepperInfo* si);
private:
  std::vector<double> _times;
  double _time_tol;
};

// Alternating stepper calls one stepper between the specified simulation
// times and another between them.
class AlternatingStepper : public Stepper
{
public:
  AlternatingStepper(Stepper* on_steps, Stepper* between_steps, std::vector<double> times, double tol);
  virtual double advance(const StepperInfo* si);
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
  MaxRatioStepper(Stepper* s, double max_ratio);
  virtual double advance(const StepperInfo* si);
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
  EveryNStepper(Stepper* s, int every_n, int offset = 0);
  virtual double advance(const StepperInfo* si);
private:
  Ptr _stepper;
  int _n;
  int _offset;
};

class ReturnPtrStepper : public Stepper
{
public:
  ReturnPtrStepper(const double * dt_store);
  virtual double advance(const StepperInfo* si);
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
  InstrumentedStepper(double * dt_store = nullptr);
  virtual ~InstrumentedStepper();
  virtual double advance(const StepperInfo* si);
  void setStepper(Stepper* s);
  double * dtPtr();
private:
  Ptr _stepper;
  double* _dt_store;
  bool _own;
};

// Uses an underlying stepper to compute dt.  If the actuall simulation-used
// previous dt was not what the underlying stepper returned the prior call to
// advance, this stepper returns/retries that dt value.
class RetryUnusedStepper : public Stepper
{
public:
  RetryUnusedStepper(Stepper* s, double tol, bool prev_prev);
  virtual double advance(const StepperInfo* si);
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
  ConstrFuncStepper(Stepper* s, std::function<double(double)> func, double max_diff);
  virtual double advance(const StepperInfo* si);
private:
  Ptr _stepper;
  std::function<double (double t)> _func;
  double _max_diff;
};



class PiecewiseStepper : public Stepper
{
public:
  PiecewiseStepper(std::vector<double> times, std::vector<double> dts, bool interpolate = true);
  virtual double advance(const StepperInfo* si);
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
  MinOfStepper(Stepper* a, Stepper* b, double tol);
  virtual double advance(const StepperInfo* si);
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
      );
  virtual double advance(const StepperInfo* si);
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
  SolveTimeAdaptiveStepper(int initial_direc, double percent_change);
  virtual double advance(const StepperInfo* si);
private:
  double _percent_change;
  int _direc;
  int _n_steps;
};

class StartupStepper : public Stepper
{
public:
  StartupStepper(Stepper* s, double initial_dt, int n_steps = 1);
  virtual double advance(const StepperInfo* si);
private:
  Ptr _stepper;
  double _dt;
  int _n;
};

class IfConvergedStepper : public Stepper
{
public:
  IfConvergedStepper(Stepper* if_converged, Stepper* if_not_converged);
  virtual double advance(const StepperInfo* si);
private:
  Ptr _converged;
  Ptr _not_converged;
};


class GrowShrinkStepper : public Stepper
{
public:
  // if source_dt != nullptr, the dt returned by it is used to grow or shrink
  // on accordingly
  GrowShrinkStepper(double shrink_factor, double growth_factor, Stepper* source_dt = nullptr);
  virtual double advance(const StepperInfo* si);
private:
  double _grow_fac;
  double _shrink_fac;
  Ptr _source_dt;
};

class PredictorCorrectorStepper : public Stepper
{
public:
  PredictorCorrectorStepper(int start_adapting, double e_tol, double scaling_param);
  virtual double advance(const StepperInfo* si);
private:
  double estimateTimeError(const StepperInfo* si);

  int _start_adapting;
  double _e_tol;
  double _scale_param;
};
