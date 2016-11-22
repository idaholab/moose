#ifndef STEPPERINFO_H
#define STEPPERINFO_H

#include <functional>

#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "libmesh/numeric_vector.h"

/// Holds all information used by Steppers to calculate dt via the "advance"
/// function.  For some variables (e.g. dt), a history of fixed-length 3 of previous
/// values is maintained.
class StepperInfo
{
public:
  StepperInfo();
  StepperInfo(const StepperInfo & si);
  StepperInfo & operator=(const StepperInfo & si);
  StepperInfo(StepperInfo&&) = delete;
  StepperInfo& operator=(StepperInfo&&) = delete;

  /// Updates internal state to match new changes to the simulation state (i.e.
  /// for a new time step). Flags relating to backupting/restore, etc. are all reset.
  void update(int step_count, Real time, Real dt, unsigned int nonlin_iters,
              unsigned int lin_iters, bool converged, Real solve_time_secs,
              std::vector<Real> soln_nonlin, std::vector<Real> soln_aux,
              std::vector<Real> soln_predicted);

  // Similar to the update function, this can be used to set up the state/history for variables
  // that support it.  This should generally NOT be called by StepperBlock implementations.
  void pushHistory(Real dt, bool converged, Real solve_time);

  /// The number of times the simulation has performed a time step iteration.
  /// This starts off equal to one.
  int stepCount();

  /// Current simulation time.
  Real time();

  /// Returns the dt used between the most recent and immediately prior solves.
  /// If n > 0, the dt used between the nth most recent and immediately prior
  /// solves is returned. n > 2 is currently not supported.
  Real dt(int n = 0);

  /// Returns the converged state of the most recent solve.  If n > 0, returns
  /// the converged state of the nth most recent solve.  n > 2 is currently not
  /// supported.
  bool converged(int n = 0);

  /// Returns the wall time taken for the most recent solve.  If n > 0, returns
  /// the wall time taken for the nth most recent solve.  n > 2 is currently not
  /// supported.
  Real solveTimeSecs(int n = 0);

  /// Number of nonlinear iterations performed for the most recent solve.
  int nonlinIters();

  /// Number of linear iterations performed for the most recent solve.
  int linIters();

  /// Nonlinear solution vector for the most recent solve.
  NumericVector<Number> * solnNonlin();

  /// Auxiliary solution vector for the most recent solve.
  NumericVector<Number> * solnAux();

  /// Predicted solution vector (if any used) for the most recent solve.
  /// If no predictor was used, this is a zero vector with the same length as
  /// soln_nonlin.
  NumericVector<Number> * solnPredicted();

  /// Instructs the executioner to backup the current simulation state
  /// *before* any upcomming dt is applied.  Save the current simulation time as
  /// the key for restoreing.
  void backup();

  /// Returns true if a backup has been requested.
  bool wantBackup();

  /// Instructs the executioner to restore the simulation to the specified
  /// target_time.  backup() must have been previously called for the
  /// target_time.  The restore does not occur when this function is called -
  /// instead it occurs when the executioner inspects the StepperInfo object.
  void restore(Real target_time);

  /// Returns the requested restore time if any has been set.  Otherwise, it
  /// returns -1.
  Real restoreTime();

private:
  int _step_count;
  Real _time;
  std::deque<Real> _dt;

  unsigned int _nonlin_iters;
  unsigned int _lin_iters;
  std::deque<bool> _converged;
  std::deque<Real> _solve_time_secs;

  std::unique_ptr<NumericVector<Number>> _soln_nonlin;
  std::unique_ptr<NumericVector<Number>> _soln_aux;
  std::unique_ptr<NumericVector<Number>> _soln_predicted;

  bool _backup;
  bool _restore;
  Real _restore_time;

  libMesh::Parallel::Communicator _dummy_comm;

  friend class Stepper;
};

#endif //STEPPERINFO_H
