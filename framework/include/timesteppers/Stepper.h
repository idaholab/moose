#pragma once

#include <functional>

#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "libmesh/numeric_vector.h"

#define MAKETOGETHER(a, b) a##b
#define DBG MAKETOGETHER(std::c, out)
#define STEPPER_LOGGING true

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

struct StepperFeedback
{
  bool snapshot;
  bool rewind;
  double rewind_time;
};

/// A base class for time stepping algorithms for use in determining dt between
/// time steps of a transient problem solution.  Implementations should strive
/// to be immutable - facilitating easier restart/recovery and testing.  Some of
/// the provided steppers, take StepperBlock* as arguments in their constructors -
/// such steppers take ownership of the passed-in steppers' memory.
class StepperBlock
{
public:
  typedef std::unique_ptr<StepperBlock> Ptr;

  virtual ~StepperBlock() = default;

  /// Returns a value for dt to calculate the next time step.  Implementations
  /// can assume si is not NULL.  Implementations of advance should strive to be
  /// idempotent.
  virtual double advance(const StepperInfo * si, StepperFeedback * sf) = 0;
};

class RootBlock : public StepperBlock
{
public:
  RootBlock(std::function<double(const StepperInfo * si)> func);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

  static StepperBlock * constant(double dt);
  static StepperBlock * prevdt();
  static StepperBlock * fixedTimes(std::vector<double> times, double tol);
  static StepperBlock * ptr(const double * dt_store);

private:
  std::function<double(const StepperInfo * si)> _func;
};

class ModBlock : public StepperBlock
{
public:
  ModBlock(StepperBlock * s, std::function<double(const StepperInfo * si, double dt)> func);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

  static StepperBlock * maxRatio(StepperBlock * s, double max_ratio);
  static StepperBlock * dtLimit(StepperBlock * s, double min, double max);
  static StepperBlock * bounds(StepperBlock * s, double t_min, double t_max);
  static StepperBlock * mult(double mult, StepperBlock * s = nullptr);

private:
  Ptr _stepper;
  std::function<double(const StepperInfo * si, double dt)> _func;
};

class IfBlock : public StepperBlock {
public:
  IfBlock(StepperBlock * on_true, StepperBlock * on_false, std::function<bool(const StepperInfo *)> func);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

  static StepperBlock * between(StepperBlock * on, StepperBlock * between, std::vector<double> times, double tol);
  static StepperBlock * everyN(StepperBlock * nth, StepperBlock * between, int every_n, int offset = 0);
  static StepperBlock * initialN(StepperBlock * initial, StepperBlock * primary, int n);
  static StepperBlock * converged(StepperBlock  * converged, StepperBlock * not_converged, bool delay = false);

private:
  Ptr _ontrue;
  Ptr _onfalse;
  std::function<bool(const StepperInfo * si)> _func;
};

/// Returns the dt of the underlying stepper unmodified.  Stores/remembers this
/// dt which can be viewed/connected to via the pointer returned by dtPtr().
/// The underlying stepper must be set via setStepper - this allows
/// InstrumentedStepper to be created before other steppers which may want to
/// use the dt_store pointer.  This stepper enables steppers at one layer of
/// nesting to base their dt calculations on dt values computed at a different
/// layer of nesting.
class InstrumentedBlock : public StepperBlock
{
public:
  InstrumentedBlock(double * dt_store = nullptr);
  virtual ~InstrumentedBlock();
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);
  void setStepper(StepperBlock * s);
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
class RetryUnusedBlock : public StepperBlock
{
public:
  /// If prev_prev is true, on retry cases, "advance" will return the last
  /// returned dt that was used (i.e. prev_prev_dt) instead of the last returned
  /// dt that was not used.
  RetryUnusedBlock(StepperBlock * s, double tol, bool prev_prev);
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
class ConstrFuncBlock : public StepperBlock
{
public:
  ConstrFuncBlock(StepperBlock * s, std::function<double(double)> func, double max_diff);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  Ptr _stepper;
  std::function<double(double t)> _func;
  double _max_diff;
};

/// Uses user-specified (time,dt) points to do a piece-wise linear interpolation
/// to calculate dt.
class PiecewiseBlock : public StepperBlock
{
public:
  /// If interpolate is false, then this finds the pair of values in the times
  /// vector that the current simulation time resides between and returns the dt
  /// at the index of the lower bound.
  PiecewiseBlock(std::vector<double> times, std::vector<double> dts, bool interpolate = true);
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
class MinOfBlock : public StepperBlock
{
public:
  /// Stepper "a" is preferred.
  MinOfBlock(StepperBlock * a, StepperBlock * b, double tol = 0);
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
class AdaptiveBlock : public StepperBlock
{
public:
  /// shrink_factor must be between 0 and 1.0.  growth_factor must be greater
  /// than or equal to 1.0.
  AdaptiveBlock(unsigned int optimal_iters, unsigned int iter_window, double lin_iter_ratio,
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
class SolveTimeAdaptiveBlock : public StepperBlock
{
public:
  /// initial_direc must be either +1.0 or -1.0 indicating whether initial
  /// adjustments to dt should be an increase or decrease.  frac_change should
  /// generally be between 0.0 and 1.0.
  SolveTimeAdaptiveBlock(int initial_direc, double frac_change);
  virtual double advance(const StepperInfo * si, StepperFeedback * sf);

private:
  double _percent_change;
  int _direc;
  int _n_steps;
};

/// Uses the error between a predictor's solution and the actual last solution
/// to compute adjustments to dt.
/// In order to use this stepper, an appropriate predictor must have been added
/// to the simulation problem.
class PredictorCorrectorBlock : public StepperBlock
{
public:
  /// start_adapting is the first si.step_count to start doing dt corrector
  /// calculations for - otherwise, prev_dt is returned by "advance".  For
  /// details on e_tol and scaling_param usage - divine it from the code.
  PredictorCorrectorBlock(int start_adapting, double e_tol, double scaling_param,
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
class DT2Block : public StepperBlock
{
public:
  DT2Block(double time_tol, double e_tol, double e_max, int integrator_order);
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

