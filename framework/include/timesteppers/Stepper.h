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
class StepperInfo
{
public:
  StepperInfo();
  StepperInfo(const StepperInfo& si);
  StepperInfo& operator=(const StepperInfo& si);
  
  void pushHistory(Real dt, bool converged, Real solve_time);

  /// Updates internal state to match new changes to the simulation state (i.e. for a new time step).
  /// Flags relating to snapshotting/rewind, etc. are all reset.
  void update(
    int step_count,
    Real time,
    Real dt,
    unsigned int nonlin_iters,
    unsigned int lin_iters,
    bool converged,
    Real solve_time_secs,
    std::vector<Real> soln_nonlin,
    std::vector<Real> soln_aux,
    std::vector<Real> soln_predicted
    );

  /// The number of times the simulation has performed a time step iteration.
  /// This starts off equal to one.
  int stepCount();

  /// Current simulation time.
  Real time();

  /// Returns the dt used between the most recent and immediately prior solves.  If n > 0, the dt used between the nth most recent and immediately prior solves is returned. n > 2 is currently not supported.
  Real dt(int n = 0);

  /// Returns the converged state of the most recent solve.  If n > 0, returns the converged state of the nth most recent solve.  n > 2 is currently not supported.
  bool converged(int n = 0);

  /// Returns the wall time taken for the most recent solve.  If n > 0, returns the wall time taken for the nth most recent solve.  n > 2 is currently not supported.
  Real solveTimeSecs(int n = 0);

  /// Number of nonlinear iterations performed for the most recent solve.
  int nonlinIters();

  /// Number of linear iterations performed for the most recent solve.
  int linIters();

  /// Nonlinear solution vector for the most recent solve.
  NumericVector<Number>* solnNonlin();

  /// Auxiliary solution vector for the most recent solve.
  NumericVector<Number>* solnAux();

  /// Predicted solution vector (if any used) for the most recent solve.
  /// If no predictor was used, this is a zero vector with the same length as
  /// soln_nonlin.
  NumericVector<Number>* solnPredicted();

  /// Instructs the executioner to snapshot the current simulation state *before* any upcomming dt is applied.  Save the current simulation time as the key for rewinding.
  void snapshot();
  
  /// Returns true if a snapshot has been requested.
  bool wantSnapshot();

  /// Instructs the executioner to rewind the simulation to the specified target_time.  snapshot() must have been previously called for the target_time.  The rewind does not occur when this function is called - instead it occurs when the executioner inspects the StepperInfo object.
  void rewind(Real target_time);

  /// Returns the requested rewind time if any has been set.  Otherwise, it returns -1.
  Real rewindTime();
  
private:
  int _step_count;
  Real _time;
  std::list<Real> _dt;

  unsigned int _nonlin_iters;
  unsigned int _lin_iters;
  std::list<bool> _converged;
  std::list<Real> _solve_time_secs;

  std::unique_ptr<NumericVector<Number>> _soln_nonlin;
  std::unique_ptr<NumericVector<Number>> _soln_aux;
  std::unique_ptr<NumericVector<Number>> _soln_predicted;
  
  bool _snapshot;
  bool _rewind;
  Real _rewind_time;
  
  libMesh::Parallel::Communicator _dummy_comm;
};

/// A base class for time stepping algorithms for use in determining dt between
/// time steps of a transient problem solution.  Implementations should strive
/// to be immutable - facilitating easier restart/recovery and testing.  Some of
/// the provided steppers, take StepperBlock* as arguments in their constructors
/// -
/// such steppers take ownership of the passed-in steppers' memory.
class StepperBlock
{
public:
  typedef std::unique_ptr<StepperBlock> Ptr;

  virtual ~StepperBlock() = default;

  /// Returns a value for dt to calculate the next time step.  Implementations
  /// can assume si is not NULL.  Implementations of advance should strive to be
  /// idempotent.
  virtual Real next(StepperInfo & si) = 0;
};

/// Holds a collection of functions for generating common, pre-configured
/// stepper block algorithms.
namespace BaseStepper
{
/// Builds a stepper that always returns the same given dt.
StepperBlock * constant(Real dt);
/// Builds a stepper that always returns the previously used simulation dt.
StepperBlock * prevdt();
/// Builds a stepper that returns dt in order to hit consecutive simulation
/// times specified in times (in ascending order).  tol is an absolute time
/// tolerance within which a time in the times vector is considered "satisfied"
/// by the current simulation time resulting in a dt to hit the next entry in
/// times.
StepperBlock * fixedTimes(std::vector<Real> times, Real tol);
/// Builds a stepper that returns the dt stored in the dt_store pointer.
StepperBlock * ptr(const Real * dt_store);
/// Builds a stepper that returns the dt from calling s->next, reducing it if
/// necessary to ensure "dt/prevdt" is less than or equal to max_ratio.
StepperBlock * maxRatio(StepperBlock * s, Real max_ratio);
/// Builds a stepper that returns the dt from calling s->next, modifying it if
/// necessary to ensure dt is between min and max.
StepperBlock * dtLimit(StepperBlock * s, Real min, Real max);
/// Builds a stepper that returns the dt from calling s->next, modifying it if
/// necessary to keep the simulation time within t_min and t_max.
StepperBlock * bounds(StepperBlock * s, Real t_min, Real t_max);
/// Builds a stepper that returns the dt from calling s->next multiplied by
/// mult.
/// If s is null, then mult is applied to the previous simulation dt.
StepperBlock * mult(Real mult, StepperBlock * s = nullptr);
/// Builds a stepper that returns the dt from calling on->next when the
/// simulation time is within tol of an entry in the times vector and returns dt
/// from calling between->next otherwise.
StepperBlock * between(StepperBlock * on, StepperBlock * between,
                       std::vector<Real> times, Real tol);
/// Builds a stepper that returns the dt from calling nth->next every every_n
/// steps and the dt val.  If between is null, then the previous simulation dt
/// is used instead.  The first call to nth->next will occur when the step count
/// is equal to offset+1.  Note that every_n is for the StepperInfo step count -
/// not for the number of calls to next.
StepperBlock * everyN(StepperBlock * nth, int every_n, int offset = 0,
                      StepperBlock * between = nullptr);
/// Builds a stepper that returns the dt from calling initial->next for the
/// first
/// n time steps (i.e. StepperInfo.step_count) and the dt from calling
/// primary->next after that.
StepperBlock * initialN(StepperBlock * initial, StepperBlock * primary, int n);
/// Builds a stepper that returns the dt from calling converged->next if the
/// last
/// solve converged and returns the dt from calling not_converged->next
/// otherwise.  If delay is true, then the solve prior to the last solve must
/// have converged as well in order to call converged->next.
StepperBlock * converged(StepperBlock * converged, StepperBlock * not_converged,
                         bool delay = false);
/// Builds a stepper that calls a->next and b->next returning the smaller dt of
/// the two.  If a and b are within absolute tol of each other, then the dt from
/// a->next is always returned.
StepperBlock * min(StepperBlock * a, StepperBlock * b, Real tol = 0);
} // namespace BaseStepper

/// Generic building block for representing steppers that calculate their own dt
/// without using other StepperBlocks - i.e. they only use StepperInfo and their
/// internal configuration.
class RootBlock : public StepperBlock
{
public:
  /// func is a (lambda) function that returns a dt using the StepperInfo object
  /// passed into a call to next.
  RootBlock(std::function<Real(StepperInfo & si)> func);
  virtual Real next(StepperInfo & si);

private:
  std::function<Real(StepperInfo & si)> _func;
};

/// Generic building block for representing steppers that return a (potentially)
/// modified version of the dt from calling next on an underlying stepper.
class ModBlock : public StepperBlock
{
public:
  /// func is a (lambda) function that modifies the passed in dt suitably before
  /// returning it.
  ModBlock(StepperBlock * s,
           std::function<Real(StepperInfo & si, Real dt)> func);
  virtual Real next(StepperInfo & si);

private:
  Ptr _stepper;
  std::function<Real(StepperInfo & si, Real dt)> _func;
};

/// Generic building block for representing steppers that call one of two
/// underlying steppers based on some condition.
class IfBlock : public StepperBlock
{
public:
  /// If func returns true, the dt from calling on_true->next is returned.
  /// Otherwise the dt from calling on_false->next is returned.  Only one of
  /// on_true or on_false is queried for dt each time step (never both).
  IfBlock(StepperBlock * on_true, StepperBlock * on_false,
          std::function<bool(StepperInfo &)> func);
  virtual Real next(StepperInfo & si);

private:
  Ptr _ontrue;
  Ptr _onfalse;
  std::function<bool(StepperInfo & si)> _func;
};

/// Returns the dt of the underlying stepper unmodified.  Stores/remembers this
/// dt which can be viewed/connected to via the pointer returned by dtPtr().
/// The underlying stepper must be set via setStepper - this allows
/// InstrumentedStepper to be created before other steppers which may want to
/// use the dt_store pointer.  InstrumentedStepper enables steppers at one layer
/// of nesting to base their dt calculations on dt values computed at a different
/// layer of nesting.
class InstrumentedBlock : public StepperBlock
{
public:
  /// dt_store is an address at which the stepper will store the dt value
  /// returned by the underlying stepper (as set by setStepper).  If null, the
  /// stepper will allocate its own storage internally and the address can be
  /// fetched via a call to dtPtr.
  InstrumentedBlock(Real * dt_store = nullptr);
  virtual ~InstrumentedBlock();
  virtual Real next(StepperInfo & si);
  void setStepper(StepperBlock * s);
  Real * dtPtr();

private:
  Ptr _stepper;
  Real * _dt_store;
  bool _own;
};

/// Uses an underlying stepper to compute dt.  If the actuall simulation-used
/// previous dt was not what the underlying stepper returned on the prior call
/// to advance and the current sim time is different than on the prior call to
/// advance, this stepper returns/retries that dt value.
class RetryUnusedBlock : public StepperBlock
{
public:
  /// If prev_prev is true, on retry cases, "advance" will return the last
  /// returned dt that was used (i.e. prev_prev_dt) instead of the last returned
  /// dt that was not used.
  RetryUnusedBlock(StepperBlock * s, Real tol, bool prev_prev);
  virtual Real next(StepperInfo & si);

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
  /// The dt from calling s->next is repeatedly divided by two until
  /// "func(t_curr + dt) - func(t_curr) <= max_diff".
  ConstrFuncBlock(StepperBlock * s, std::function<Real(Real)> func,
                  Real max_diff);
  virtual Real next(StepperInfo & si);

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
  PiecewiseBlock(std::vector<Real> times, std::vector<Real> dts,
                 bool interpolate = true);
  virtual Real next(StepperInfo & si);

private:
  std::vector<Real> _times;
  std::vector<Real> _dts;
  bool _interp;
  LinearInterpolation _lin;
};

/// Returns the smaller of two dt's from two underlying steppers (one of
/// them preferred).  It returns the preferred stepper's dt if
/// "dt_preferred - tolerance < dt_alternate". Otherwise it returns the
/// alternate stepper's dt.
class MinOfBlock : public StepperBlock
{
public:
  /// Stepper "a" is preferred.
  MinOfBlock(StepperBlock * a, StepperBlock * b, Real tol);
  virtual Real next(StepperInfo & si);

private:
  Ptr _a;
  Ptr _b;
  Real _tol;
};

/// Computes dt adaptively based on the number of linear and non-linear
/// iterations that were required to converge on the most recent solve.  Too
/// many iterations results in dt contraction, too few iterations results in dt
/// growth.  For algorithm details, read the code.
class AdaptiveBlock : public StepperBlock
{
public:
  /// shrink_factor must be between 0 and 1.0.  growth_factor must be greater
  /// than or equal to 1.0.
  AdaptiveBlock(unsigned int optimal_iters, unsigned int iter_window,
                Real lin_iter_ratio, Real shrink_factor, Real growth_factor);
  virtual Real next(StepperInfo & si);

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
  virtual Real next(StepperInfo & si);

private:
  Real _percent_change;
  int _direc;
  int _n_steps;
};

/// Uses the error between a predictor's solution and the actual last solution
/// to compute adjustments to dt.  In order to use this stepper, an
/// appropriate predictor must have been added to the simulation problem.
class PredictorCorrectorBlock : public StepperBlock
{
public:
  /// start_adapting is the first si.step_count to start doing dt corrector
  /// calculations for - otherwise, prev_dt is returned by "advance".  For
  /// details on e_tol and scaling_param usage - divine it from the code.
  PredictorCorrectorBlock(int start_adapting, Real e_tol, Real scaling_param,
                          std::string time_integrator);
  virtual Real next(StepperInfo & si);

private:
  Real estimateTimeError(StepperInfo & si);

  int _start_adapting;
  Real _e_tol;
  Real _scale_param;
  std::string _time_integrator;
};

/// Calculates dt by directing a sequence of solves that divide the region
/// between times a and b into first one time step starting at t=a (dt=b-a) and
/// then two time steps starting at t=a (dt=(b-a)/2).  The norm of the
/// difference between the two resulting solutions at t=b is used to adjust the
/// time step to determine a new a/b time window.
class DT2Block : public StepperBlock
{
public:
  /// Simulation time is considered at the a/b time window end/mid points if it
  /// is within time_tol of the calculated values for those times.  e_tol is the
  /// target error between the smaller and larger time step solutions.  e_max is
  /// the error between the smaller and larger time step solutions above which
  /// the solution is considered not converged and a rewind is performed to the
  /// start of the window and the b endpoint of the window is shrunk by a factor
  /// of two.
  DT2Block(Real time_tol, Real e_tol, Real e_max, int integrator_order);
  virtual Real next(StepperInfo & si);

private:
  Real dt();
  Real resetWindow(Real start, Real dt);
  Real calcErr(StepperInfo & si);
  Real _tol;
  Real _e_tol;
  Real _e_max;
  int _order;
  Real _start_time;
  Real _end_time;
  std::unique_ptr<NumericVector<Number>> _big_soln;
};

#endif // STEPPERBLOCK_H
