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

#ifndef STEPPERBLOCK_H
#define STEPPERBLOCK_H

#include <functional>

#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "libmesh/numeric_vector.h"

/// Holds all information used by Steppers to calculate dt via the "advance"
/// function.
struct StepperInfo
{
  /// The number of times the simulation has requested a dt.
  /// This is generally equal to one on the first call fo next(...).
  int step_count;

  /// Current simulation time.
  Real time;
  /// dt used between solves for the most and second most recent time steps.
  Real prev_dt;
  /// dt used between solves for the second most and third most recent time
  /// steps.
  Real prev_prev_dt;
  /// dt used between solves for the third most and fourth most recent time
  /// steps.
  Real prev_prev_prev_dt;

  /// Number of nonlinear iterations performed for the most recent solve.
  unsigned int nonlin_iters;
  /// Number of linear iterations performed for the most recent solve.
  unsigned int lin_iters;
  /// Whether or not the solve for the most recent time step converged.
  bool converged;
  /// Whether or not the solve for the second most recent time step converged.
  bool prev_converged;
  /// Clock time used in the most recent time step/solve.
  Real prev_solve_time_secs;
  /// Clock time used in the second most recent time step/solve.
  Real prev_prev_solve_time_secs;
  /// Clock time used in the third most recent time step/solve.
  Real prev_prev_prev_solve_time_secs;

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
  Real rewind_time;
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
  virtual Real next(const StepperInfo & si, StepperFeedback & sf) = 0;
};

namespace BaseStepper
{
StepperBlock * constant(Real dt);
StepperBlock * prevdt();
StepperBlock * fixedTimes(std::vector<Real> times, Real tol);
StepperBlock * ptr(const Real * dt_store);
StepperBlock * maxRatio(StepperBlock * s, Real max_ratio);
StepperBlock * dtLimit(StepperBlock * s, Real min, Real max);
StepperBlock * bounds(StepperBlock * s, Real t_min, Real t_max);
StepperBlock * mult(Real mult, StepperBlock * s = nullptr);
StepperBlock * between(StepperBlock * on, StepperBlock * between, std::vector<Real> times, Real tol);
StepperBlock * everyN(StepperBlock * nth, int every_n, int offset = 0, StepperBlock * between = nullptr);
StepperBlock * initialN(StepperBlock * initial, StepperBlock * primary, int n);
StepperBlock * converged(StepperBlock * converged, StepperBlock * not_converged, bool delay = false);
StepperBlock * min(StepperBlock * a, StepperBlock * b, Real tol = 0);
} // namespace BaseStepper

class RootBlock : public StepperBlock
{
public:
  RootBlock(std::function<Real(const StepperInfo & si)> func);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  std::function<Real(const StepperInfo & si)> _func;
};

class ModBlock : public StepperBlock
{
public:
  ModBlock(StepperBlock * s, std::function<Real(const StepperInfo & si, Real dt)> func);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  Ptr _stepper;
  std::function<Real(const StepperInfo & si, Real dt)> _func;
};

class IfBlock : public StepperBlock
{
public:
  IfBlock(StepperBlock * on_true, StepperBlock * on_false, std::function<bool(const StepperInfo &)> func);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  Ptr _ontrue;
  Ptr _onfalse;
  std::function<bool(const StepperInfo & si)> _func;
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
  InstrumentedBlock(Real * dt_store = nullptr);
  virtual ~InstrumentedBlock();
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);
  void setStepper(StepperBlock * s);
  Real * dtPtr();

private:
  Ptr _stepper;
  Real * _dt_store;
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
  RetryUnusedBlock(StepperBlock * s, Real tol, bool prev_prev);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  Ptr _stepper;
  Real _tol;
  bool _prev_prev;
  Real _prev_dt;
  Real _prev_time;
};

/// ConstrFuncStepper reduces the returned dt of an underlying stepper by
/// factors of two until the difference between a given limiting function
/// evaluated at t_curr and t_next is less than a specified maximum difference.
class ConstrFuncBlock : public StepperBlock
{
public:
  ConstrFuncBlock(StepperBlock * s, std::function<Real(Real)> func, Real max_diff);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  Ptr _stepper;
  std::function<Real(Real t)> _func;
  Real _max_diff;
};

/// Uses user-specified (time,dt) points to do a piece-wise linear interpolation
/// to calculate dt.
class PiecewiseBlock : public StepperBlock
{
public:
  /// If interpolate is false, then this finds the pair of values in the times
  /// vector that the current simulation time resides between and returns the dt
  /// at the index of the lower bound.
  PiecewiseBlock(std::vector<Real> times, std::vector<Real> dts, bool interpolate = true);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  std::vector<Real> _times;
  std::vector<Real> _dts;
  bool _interp;
  LinearInterpolation _lin;
};

/// Returns the smaller of two dt's from two underlying steppers
/// (one of them preferred).
/// It returns the preferred stepper's dt if "dt_preferred - tolerance <
/// dt_alternate".
/// Otherwise it returns the alternate stepper's dt.
class MinOfBlock : public StepperBlock
{
public:
  /// Stepper "a" is preferred.
  MinOfBlock(StepperBlock * a, StepperBlock * b, Real tol);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  Ptr _a;
  Ptr _b;
  Real _tol;
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
  AdaptiveBlock(unsigned int optimal_iters, unsigned int iter_window, Real lin_iter_ratio,
                Real shrink_factor, Real growth_factor);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  unsigned int _optimal_iters;
  unsigned int _iter_window;
  Real _lin_iter_ratio;
  Real _shrink_factor;
  Real _growth_factor;
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
  SolveTimeAdaptiveBlock(int initial_direc, Real frac_change);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  Real _percent_change;
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
  PredictorCorrectorBlock(int start_adapting, Real e_tol, Real scaling_param,
                          std::string time_integrator);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  Real estimateTimeError(const StepperInfo & si);

  int _start_adapting;
  Real _e_tol;
  Real _scale_param;
  std::string _time_integrator;
};

// TODO: need mechanism for telling executioner to not output data for "intermediate" solve/time steps
class DT2Block : public StepperBlock
{
public:
  DT2Block(Real time_tol, Real e_tol, Real e_max, int integrator_order);
  virtual Real next(const StepperInfo & si, StepperFeedback & sf);

private:
  Real dt();
  Real resetWindow(Real start, Real dt);
  Real calcErr(const StepperInfo & si);
  Real _tol;
  Real _e_tol;
  Real _e_max;
  int _order;
  Real _start_time;
  Real _end_time;
  std::unique_ptr<NumericVector<Number>> _big_soln;
};

#endif //STEPPERBLOCK_H
