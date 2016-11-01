#pragma once

#include <functional>

#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "libmesh/numeric_vector.h"

#define MAKETOGETHER(a, b) a##b
#define DBG MAKETOGETHER(std::c, out)
#define STEPPER_LOGGING true

class Logger
{
public:
  Logger(std::string loc) : _loc(loc), _dt(-1)
  {
    if (on)
    {
      DBG << "[LOG] " << std::string(level * 4, ' ') << "- entering " << _loc << "\n";
      level++;
    }
  }
  ~Logger()
  {
    if (on)
    {
      level--;
      DBG << "[LOG] " << std::string(level * 4, ' ') << "- leaving " << _loc << " (dt=" << _dt
          << ")\n";
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

struct StepperFeedback
{
  bool snapshot;
  bool rewind;
  double rewind_time;
};

/// Holds all information used by Steppers to calculate dt via the "advance"
/// function.
struct StepperInfo
{
  /// The number of times the simulation has requested a dt.
  /// This is generally equal to one on the first call fo advance(...).
  int step_count;

  /// Current simulation time.
  double time;
  /// dt used between solves for the most and second most recent time steps.
  double prev_dt;
  /// dt used between solves for the second most and third most recent time
  /// steps.
  double prev_prev_dt;
  /// dt used between solves for the third most and fourth most recent time
  /// steps.
  double prev_prev_prev_dt;

  /// Number of nonlinear iterations performed for the most recent solve.
  unsigned int nonlin_iters;
  /// Number of linear iterations performed for the most recent solve.
  unsigned int lin_iters;
  /// Whether or not the solve for the most recent time step converged.
  bool converged;
  /// Whether or not the solve for the second most recent time step converged.
  bool prev_converged;
  /// Clock time used in the most recent time step/solve.
  double prev_solve_time_secs;
  /// Clock time used in the second most recent time step/solve.
  double prev_prev_solve_time_secs;
  /// Clock time used in the third most recent time step/solve.
  double prev_prev_prev_solve_time_secs;

  /// Nonlinear solution vector for the most recent solve.
  std::unique_ptr<NumericVector<Number>> soln_nonlin;
  /// Auxiliary solution vector for the most recent solve.
  std::unique_ptr<NumericVector<Number>> soln_aux;
  /// Predicted solution vector (if any used) for the most recent solve.
  /// If no predictor was used, this is a zero vector with the same length as
  /// soln_nonlin.
  std::unique_ptr<NumericVector<Number>> soln_predicted;
};

/// A base class for time stepping algorithms for use in determining dt between
/// time steps of a transient problem solution.  Implementations should strive
/// to be immutable - facilitating easier restart/recovery and testing.  Some of
/// the provided steppers, take Stepper* as arguments in their constructors -
/// such steppers take ownership of the passed-in steppers' memory.
class Stepper
{
public:
  typedef std::unique_ptr<Stepper> Ptr;

  virtual ~Stepper() = default;

  /// Returns a value for dt to calculate the next time step.  Implementations
  /// can assume si is not NULL.  Implementations of advance should strive to be
  /// idempotent.
  virtual double advance(const StepperInfo * si, StepperFeedback * sf) = 0;
};

/// Always returns a single, unchanging, user-specified dt.
class ConstStepper : public Stepper
{
public:
  ConstStepper(double dt);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  double _dt;
};

/// Calls an underlying stepper to compute dt. Modifies this dt to be within
/// specified fixed lower and upper bounds (or throws an error - depending on
/// configuration).
class DTLimitStepper : public Stepper
{
public:
  /// If throw_err is true, an exception is thrown if the underlying stepper's
  /// advance function returns an out of bounds dt.  Otherwise, out-of-bounds
  /// cases result in dt being set to the corresponding min or max bound.
  DTLimitStepper(Stepper * s, double dt_min, double dt_max, bool throw_err);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _stepper;
  double _min;
  double _max;
  bool _err;
};

/// Calls an underlying stepper to compute dt. Modifies this dt to maintain the
/// simulation time within specified fixed lower and upper bounds (or throws an
/// error - depending on configuration).
class BoundsStepper : public Stepper
{
public:
  /// If throw_err is true, an exception is thrown if the underlying stepper's
  /// advance function returns a dt that would cause an out of bounds simulation
  /// time.  Otherwise, out-of-bounds cases result in dt being set to hit the
  /// corresponding time bound.
  BoundsStepper(Stepper * s, double t_min, double t_max, bool throw_err);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _stepper;
  double _min;
  double _max;
  bool _err;
};

/// Calculates dt in order to hit specified fixed times. This is robust if the
/// dt used to evolve the actual simulation ended up being changed from this
/// dt.  Returns infinity after the simulation time is beyond the last
/// user-specified time.
class FixedPointStepper : public Stepper
{
public:
  FixedPointStepper(std::vector<double> times, double tol);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  std::vector<double> _times;
  double _time_tol;
};

/// Alternating stepper calls one stepper between the specified simulation
/// times and the other stepper on the specified simulation times.
class AlternatingStepper : public Stepper
{
public:
  AlternatingStepper(Stepper * on_steps, Stepper * between_steps, std::vector<double> times,
                     double tol);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _on_steps;
  Ptr _between_steps;
  std::vector<double> _times;
  double _time_tol;
};

/// Calls an underlying stepper to compute dt.  If this dt
/// violates the expression  "dt / prev_dt <= max_ratio", then
/// this stepper returns dt modified to be equal to "prev_dt * max_ratio".
class MaxRatioStepper : public Stepper
{
public:
  MaxRatioStepper(Stepper * s, double max_ratio);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _stepper;
  double _max_ratio;
};

/// Delegates dt calculations to an underlying stepper only every N steps (not
/// every n calls to advance).
/// Otherwise, it returns the previous dt.  If an offset is provided, the first
/// call to advance will occur on the offset'th step.
class EveryNStepper : public Stepper
{
public:
  // offset must be smaller than every_n.
  EveryNStepper(Stepper * s, int every_n, int offset = 0);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _stepper;
  int _n;
  int _offset;
};

/// Returns the dt value stored in the address it was initialized with.
class ReturnPtrStepper : public Stepper
{
public:
  /// dt_store must not be null.
  ReturnPtrStepper(const double * dt_store);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  const double * _dt_store;
};

/// Returns the dt of the underlying stepper unmodified.  Stores/remembers this
/// dt which can be viewed/connected to via the pointer returned by dtPtr().
/// The underlying stepper must be set via setStepper - this allows
/// InstrumentedStepper to be created before other steppers which may want to
/// use the dt_store pointer.  This stepper enables steppers at one layer of
/// nesting to base their dt calculations on dt values computed at a different
/// layer of nesting.
class InstrumentedStepper : public Stepper
{
public:
  InstrumentedStepper(double * dt_store = nullptr);
  virtual ~InstrumentedStepper();
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);
  void setStepper(Stepper * s);
  double * dtPtr();

private:
  Ptr _stepper;
  double * _dt_store;
  bool _own;
};

/// Uses an underlying stepper to compute dt.  If the actuall simulation-used
/// previous dt was not what the underlying stepper returned on the prior call
/// to advance and the current sim time is different than on the prior call to advance,
/// this stepper returns/retries that dt value.
class RetryUnusedStepper : public Stepper
{
public:
  /// If prev_prev is true, on retry cases, "advance" will return the last
  /// returned dt that was used (i.e. prev_prev_dt) instead of the last returned
  /// dt that was not used.
  RetryUnusedStepper(Stepper * s, double tol, bool prev_prev);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _stepper;
  double _tol;
  bool _prev_prev;
  double _prev_dt;
  double _prev_time;
};

/// ConstrFuncStepper reduces the returned dt of an underlying stepper by
/// factors of two until the difference between a given limiting function
/// evaluated at t_curr and t_next is less than a specified maximum difference.
class ConstrFuncStepper : public Stepper
{
public:
  ConstrFuncStepper(Stepper * s, std::function<double(double)> func, double max_diff);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _stepper;
  std::function<double(double t)> _func;
  double _max_diff;
};

/// Uses user-specified (time,dt) points to do a piece-wise linear interpolation
/// to calculate dt.
class PiecewiseStepper : public Stepper
{
public:
  /// If interpolate is false, then this finds the pair of values in the times
  /// vector that the current simulation time resides between and returns the dt
  /// at the index of the lower bound.
  PiecewiseStepper(std::vector<double> times, std::vector<double> dts, bool interpolate = true);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  std::vector<double> _times;
  std::vector<double> _dts;
  bool _interp;
  LinearInterpolation _lin;
};

/// MinOfStepper returns the smaller of two dt's from two underlying steppers
/// (one of them preferred).
/// It returns the preferred stepper's dt if "dt_preferred - tolerance <
/// dt_alternate".
/// Otherwise it returns the alternate stepper's dt.
class MinOfStepper : public Stepper
{
public:
  /// Stepper "a" is preferred.
  MinOfStepper(Stepper * a, Stepper * b, double tol);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _a;
  Ptr _b;
  double _tol;
};

/// Computes dt adaptively based on the number of linear and non-linear
/// iterations that were required to converge on the most recent solve.  Too
/// many iterations results in dt contraction, too few iterations results in dt
/// growth.
/// For algorithm details, read the code.
class AdaptiveStepper : public Stepper
{
public:
  /// shrink_factor must be between 0 and 1.0.  growth_factor must be greater
  /// than or equal to 1.0.
  AdaptiveStepper(unsigned int optimal_iters, unsigned int iter_window, double lin_iter_ratio,
                  double shrink_factor, double growth_factor);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  unsigned int _optimal_iters;
  unsigned int _iter_window;
  double _lin_iter_ratio;
  double _shrink_factor;
  double _growth_factor;
};

/// Adjusts dt in order to minimize the quantity "solve_time / prev_dt" by
/// multiplying dt by "1 +/- frac_change" if the ratio increases on consecutive
/// time steps.
class SolveTimeAdaptiveStepper : public Stepper
{
public:
  /// initial_direc must be either +1.0 or -1.0 indicating whether initial
  /// adjustments to dt should be an increase or decrease.  frac_change should
  /// generally be between 0.0 and 1.0.
  SolveTimeAdaptiveStepper(int initial_direc, double frac_change);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  double _percent_change;
  int _direc;
  int _n_steps;
};

/// Returns an initial, user-specified dt until the si.step_count exceeds a
/// specified number (not a number of calls to "advance") after which it returns
/// the dt calculated by an underlying stepper.
class StartupStepper : public Stepper
{
public:
  /// n_steps is the number of steps to return initial_dt before defaulting to
  /// returning dt from s.
  StartupStepper(Stepper * s, double initial_dt, int n_steps = 1);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _stepper;
  double _dt;
  int _n;
};

/// Calls one underlying stepper to calculate dt if the last solve converged,
/// and another otherwise.
class IfConvergedStepper : public Stepper
{
public:
  /// if_converged and if_not_converged must not be NULL.  If delay is true, then if_converged will only be called if the most recent *two* solves converged.
  IfConvergedStepper(Stepper * if_converged, Stepper * if_not_converged, bool delay = false);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _converged;
  Ptr _not_converged;
  bool _delay;
};

/// If the solve converged, multiply the previous dt by a growth factor,
/// otherwise multiply it by a shrink factor.
class GrowShrinkStepper : public Stepper
{
public:
  /// shrink_factor must be between 0.0 and 1.0.  growth_factor must be greater
  /// than or equal to 1.0.
  /// If source_dt != nullptr, the dt returned by source_dt is used to grow or
  /// shrink
  /// on instead of the previous dt.
  GrowShrinkStepper(double shrink_factor, double growth_factor, Stepper * source_dt = nullptr);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  double _grow_fac;
  double _shrink_fac;
  Ptr _source_dt;
};

/// Uses the error between a predictor's solution and the actual last solution
/// to compute adjustments to dt.
/// In order to use this stepper, an appropriate predictor must have been added
/// to the simulation problem.
class PredictorCorrectorStepper : public Stepper
{
public:
  /// start_adapting is the first si.step_count to start doing dt corrector
  /// calculations for - otherwise, prev_dt is returned by "advance".  For
  /// details on e_tol and scaling_param usage - divine it from the code.
  PredictorCorrectorStepper(int start_adapting, double e_tol, double scaling_param,
                            std::string time_integrator);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  double estimateTimeError(const StepperInfo * si);

  int _start_adapting;
  double _e_tol;
  double _scale_param;
  std::string _time_integrator;
};

// TODO: need mechanism for telling executioner to not output data for "intermediate" solve/time steps
class DT2Stepper : public Stepper
{
public:
  DT2Stepper(double time_tol, double e_tol, double e_max, int integrator_order);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  double dt();
  double resetWindow(double start, double dt);
  double calcErr(const StepperInfo * si);
  double _tol;
  double _e_tol;
  double _e_max;
  int _order;
  double _start_time;
  double _end_time;
  std::unique_ptr<NumericVector<Number>> _big_soln;
};
