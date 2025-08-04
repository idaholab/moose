//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executioner.h"
#include "TimeIntegrator.h"

// System includes
#include <string>
#include <fstream>

class TimeStepper;
class FEProblemBase;

/**
 * Base class for transient executioners that use a FixedPointSolve solve object for multiapp-main
 * app iterations
 */
class TransientBase : public Executioner
{
public:
  static InputParameters defaultSteadyStateConvergenceParams();
  static InputParameters validParams();

  TransientBase(const InputParameters & parameters);

  virtual void init() override;

  virtual void execute() override;

  /**
   * Do whatever is necessary to advance one step.
   */
  virtual void takeStep(Real input_dt = -1.0);

  /**
   * @return The fully constrained dt for this timestep
   */
  virtual Real computeConstrainedDT();
  virtual void estimateTimeError();

  /**
   * @return The the computed dt to use for this timestep.
   */
  virtual Real getDT();

  /**
   * Transient loop will continue as long as this keeps returning true.
   */
  virtual bool keepGoing();

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged() const override;

  virtual void preExecute() override;

  virtual void postExecute() override;

  virtual void computeDT();

  virtual void preStep();

  virtual void postStep();

  /**
   * This is where the solve step is actually incremented.
   */
  virtual void incrementStepOrReject();

  virtual void endStep(Real input_time = -1.0);

  /**
   * Can be used to set the next "target time" which is a time to nail perfectly.
   * Useful for driving MultiApps.
   */
  virtual void setTargetTime(Real target_time);

  /**
   * Get the current time.
   */
  virtual Real getTime() const { return _time; };

  /**
   * Get the current target time
   * @return target time
   */
  virtual Real getTargetTime() { return _target_time; }

  /**
   * Set the current time.
   */
  virtual void setTime(Real t) { _time = t; };

  /**
   * Set the old time.
   */
  virtual void setTimeOld(Real t) { _time_old = t; };

  /**
   * Compute the relative L2 norm of the change in the solution.
   */
  Real computeSolutionChangeNorm() const;
  Real computeSolutionChangeNorm(bool check_aux, bool normalize_by_dt) const;

  /**
   * The relative L2 norm of the difference between solution and old solution vector.
   */
  Real relativeSolutionDifferenceNorm() const;
  virtual Real relativeSolutionDifferenceNorm(bool check_aux) const = 0;

  /**
   * Pointer to the TimeStepper
   * @return Pointer to the time stepper for this Executioner
   */
  TimeStepper * getTimeStepper() { return _time_stepper; }
  const TimeStepper * getTimeStepper() const { return _time_stepper; }

  /**
   * Set the timestepper to use.
   *
   * @param ts The TimeStepper to use
   */
  void setTimeStepper(TimeStepper & ts);

  /**
   * Get the name of the timestepper.
   */
  virtual std::string getTimeStepperName() const override;

  /**
   * Get the time integrators (time integration scheme) used
   * Note that because some systems might be steady state simulations, there could be less
   * time integrators than systems
   * @return string with the time integration scheme name
   */
  virtual std::set<TimeIntegrator *> getTimeIntegrators() const = 0;

  /**
   * Get the name of the time integrator (time integration scheme) used
   * @return string with the time integration scheme name
   */
  virtual std::vector<std::string> getTimeIntegratorNames() const override;

  /**
   * Get the time scheme used
   * @return MooseEnum with the time scheme
   */
  Moose::TimeIntegratorType getTimeScheme() const { return _time_scheme; }

  /**
   * Get the set of sync times
   * @return The reference to the set of sync times
   */
  std::set<Real> & syncTimes() { return _sync_times; }

  /**
   * Get the maximum dt
   * @return The maximum dt
   */
  Real & dtMax() { return _dtmax; }

  /**
   * Get the minimal dt
   * @return The minimal dt
   */
  Real & dtMin() { return _dtmin; }

  /**
   * Return the start time
   * @return The start time
   */
  Real getStartTime() const { return _start_time; }

  /**
   * Get the end time
   * @return The end time
   */
  Real getEndTime() const { return _end_time; }

  /**
   * Get a modifiable reference to the end time
   * @return The end time
   */
  Real & endTime() { return _end_time; }

  /**
   * Get the timestep tolerance
   * @return The timestep tolerance
   */
  Real & timestepTol() { return _timestep_tolerance; }

  /**
   * Set the timestep tolerance
   * @param tolerance timestep tolerance
   */
  virtual void setTimestepTolerance(const Real & tolerance) { _timestep_tolerance = tolerance; }

  /**
   * Is the current step at a sync point (sync times, time interval, target time, etc)?
   * @return Bool indicataing whether we are at a sync point
   */
  bool atSyncPoint() { return _at_sync_point; }

  /**
   * Get the unconstrained dt
   * @return Value of dt before constraints were applied
   */
  Real unconstrainedDT() { return _unconstrained_dt; }

  void parentOutputPositionChanged() override;

  /**
   * Set the number of time steps
   * @param num_steps number of time steps
   */
  virtual void forceNumSteps(const unsigned int num_steps) { _num_steps = num_steps; }

  /// Return the solve object wrapped by time stepper
  virtual SolveObject * timeStepSolveObject() { return _fixed_point_solve.get(); }

protected:
  /// Here for backward compatibility
  FEProblemBase & _problem;

  /// Reference to auxiliary system base for faster access
  AuxiliarySystem & _aux;

  Moose::TimeIntegratorType _time_scheme;
  TimeStepper * _time_stepper;

  /// Current timestep.
  int & _t_step;
  /// Current time
  Real & _time;
  /// Previous time
  Real & _time_old;
  /// Current delta t... or timestep size.
  Real & _dt;
  Real & _dt_old;

  Real & _unconstrained_dt;
  bool & _at_sync_point;

  /// Whether or not the last solve converged
  bool & _last_solve_converged;

  /// Whether step should be repeated due to xfem modifying the mesh
  bool _xfem_repeat_step;

  Real _end_time;
  Real _dtmin;
  Real _dtmax;
  unsigned int _num_steps;
  int _n_startup_steps;

  /**
   * Steady state detection variables:
   */
  const bool _steady_state_detection;
  const Real _steady_state_start_time;

  std::set<Real> & _sync_times;

  bool _abort;
  /// This parameter controls how the system will deal with _dt <= _dtmin
  /// If true, the time stepper is expected to throw an error
  /// If false, the executioner will continue through EXEC_FINAL
  const bool _error_on_dtmin;

  ///if to use time interval output
  bool & _time_interval;
  Real _next_interval_output_time;
  Real _time_interval_output_interval;

  Real _start_time;
  Real _timestep_tolerance;
  Real & _target_time;
  bool _use_multiapp_dt;

  void setupTimeIntegrator();

  /// Determines whether the problem has converged to steady state
  bool convergedToSteadyState() const;

private:
  /// Constrain the timestep dt_cur by looking at the timesteps for the MultiApps on execute_on
  void constrainDTFromMultiApp(Real & dt_cur,
                               std::ostringstream & diag,
                               const ExecFlagType & execute_on) const;

  /// Whether to use the auxiliary system solution to determine steady-states
  const bool _check_aux;

  /// Whether to divide the solution difference norm by dt. If taking 'small' time steps this member
  /// should probably be true. If taking very 'large' timesteps in an attempt to reach a
  /// steady-state, this member should probably be be false.
  const bool _normalize_solution_diff_norm_by_dt;
};
