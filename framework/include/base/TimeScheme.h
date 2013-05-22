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

#ifndef TIMESCHEME_H
#define TIMESCHEME_H

#include "NonlinearSystem.h"
#include "Conversion.h"

// libMesh includes
#include "libmesh/numeric_vector.h"
#include "libmesh/petsc_vector.h"

class NonlinearSystem;

class TimeScheme
{
  /**
   * Forward declaration of TimeStep helper class.
   * Full declaration is below.
   */
  class TimeStep;

public:
  TimeScheme(NonlinearSystem * c);

  virtual ~TimeScheme();

  /**
   * Modify the initial solution vector to apply a predictor
   * @param initial_solution The initial solution vector
   */
  void applyPredictor(NumericVector<Number> & initial_solution);

  /**
   * Called at the beginning of the time step
   */
  void onTimestepBegin();

  void rejectStep();

  /**
   * Second Order Adams_Bashforth Predictor, takes as argument the initial solution
   * Currently needed for getting the errors using estimate error.
   * Used by calling Predictor after calling useAB2Preditor,
   * This should change if Time Schemes get registered or
   * something simliar.
   */
  void Adams_Bashforth2P(NumericVector<Number> & initial_solution);

  virtual NumericVector<Number> & solutionUDot() { return _solution_u_dot; }
  virtual NumericVector<Number> & solutionDuDotDu() { return _solution_du_dot_du; }
public:
  /**
   * Estimates the Time Error based on BDF2 (or Crank-Nicolson) and AB2
   * (Crank-Nicolson has issues that BDF2 does not)
   */
  Real estimateTimeError(NumericVector<Number> & solution);

  void timeSteppingScheme(Moose::TimeSteppingScheme scheme)
  {
    _time_stepping_scheme = scheme;
  }

  /**
   * Gets the time-stepping scheme
   * @return Time-stepping scheme being used
   */
  Moose::TimeSteppingScheme timeSteppingScheme() { return _time_stepping_scheme; }

  /**
   *  Used for setting if Adams-Bashforth is to be used, will go away if
   *  Time Schemes are registered or something like that.
   */
  void useAB2Predictor()
  {
    _use_predictor = true;
    _use_AB2 = true;
  }

  void setPredictorScale(Real scale)
  {
    _use_predictor = true;
    _apply_predictor = true;
    _predictor_scale = scale;
  }

  void computeLittlef(const NumericVector<Number> & bigF, NumericVector<Number> & littlef, Real time = -1, bool mass = true);
  /**
   * Computes the time derivative vector
   */
  void computeTimeDerivatives();
  void setSolutionDuDotDu(Real value);

  /**
   * Completes the assembly of residual
   * @param residual[out] Residual is formed here
   */
  void finishResidual(NumericVector<Number> & residual);

  bool _use_AB2;
  bool _use_littlef;
  ///Currently set to true, this is for AB2 in case one wishes to estimate the pridictor but not take the step.
  bool _apply_predictor;
protected:




  NonlinearSystem * _nl;

  /// solution vector for u^dot
  NumericVector<Number> & _solution_u_dot;

  /// solution vector for {du^dot}\over{du}
  NumericVector<Number> & _solution_du_dot_du;

  /// residual evaluated at the old time step (need for Crank-Nicolson)
  NumericVector<Number> & _residual_old;
  NumericVector<Number> & _predicted_solution;

  NumericVector<Number> & _tmp_previous_solution;
  NumericVector<Number> & _tmp_residual_old;
  NumericVector<Number> & _tmp_solution_u_dot;
  NumericVector<Number> & _scaled_update;
  NumericVector<Number> & _mmatrix;
  /// time
  /// size of the time step
  Real & _dt;
  /// previous time step size
  Real & _dt_old;
  /// Coefficients (weights) for the time discretization
  std::vector<Real> & _time_weight;
  /// Time stepping scheme used for time discretization
  Moose::TimeSteppingScheme _time_stepping_scheme;
  int & _t_step;
  Real & _t;
  /// true if predictor is active
  bool _use_predictor;
  /// Scale factor to use with predictor
  Real _predictor_scale;

  ///deque might be better to use than vector, forgetting the bottom of the stack to minimize memory usage might
  ///be good. Removing the bottom is not in the code currently.
  std::deque<TimeStep> _time_stack;

  std::vector<NumericVector<Number> *> _workvecs;

  // moves vectors contained in TimeStep into _workvecs, libMesh::System manages backing memory
  void reclaimTimeStep(TimeStep &timestep);
  void firstOrderTD();



  ///default to false, used to determine if AB2 predictor should be used
  TimeStep *_dt2_check;
  bool _dt2_bool;
  int _time_stack_size;
};



/**
 * Full declaration of the TimeStep helper class.
 */
class TimeScheme::TimeStep
{
public:
  TimeStep(Real time, int t_step, NonlinearSystem * nl, std::vector<NumericVector<Number> *> workvecs);
  TimeStep(NonlinearSystem * nl );
  TimeStep(const TimeStep & p);

  ~TimeStep() {}

  int getTimeStep() { return _t_step; }

  void setTime(Real & t) { _time = t; }

  Real getTime() { return _time; }

  void setSolution(NumericVector<Number> & num) { num.localize(*_solution); }
  void setTimeDerivative(NumericVector<Number> & num) { num.localize(*_time_derivative); }

  NumericVector<Number> & getSolution() { return *_solution; }

  Real getDt() { return _dt; }
  void setDt(Real & dt) { _dt=dt; }
  NumericVector<Number> & getTimeDerivative() { return *_time_derivative; }

protected:
  NonlinearSystem * _nl;
  Real _time;
  int _t_step;
  NumericVector<Number> * _solution;
  /// This is necessary currently, is storing this better or worse than otherwise?
  NumericVector<Number> * _time_derivative;
  Real _dt;
};

#endif /* TIMESCHEME_H */
