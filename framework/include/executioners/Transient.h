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

#ifndef TRANSIENT_H
#define TRANSIENT_H

#include "Executioner.h"

// System includes
#include <string>
#include <fstream>

// Forward Declarations
class Transient;
class TimeStepper;
class FEProblemBase;

template <>
InputParameters validParams<Transient>();

/**
 * Transient executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class Transient : public Executioner
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Transient(const InputParameters & parameters);

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
  virtual bool lastSolveConverged() override;

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
  virtual Real getTime() { return _time; };

  /**
   * Set the current time.
   */
  virtual void setTime(Real t) { _time = t; };

  /**
   * Set the old time.
   */
  virtual void setTimeOld(Real t) { _time_old = t; };

  /**
   * Get the Relative L2 norm of the change in the solution.
   */
  Real getSolutionChangeNorm();

  /**
   * Pointer to the TimeStepper
   * @return Pointer to the time stepper for this Executioner
   */
  TimeStepper * getTimeStepper() { return _time_stepper.get(); }

  /**
   * Set the timestepper to use.
   *
   * @param ts The TimeStepper to use
   */
  void setTimeStepper(std::shared_ptr<TimeStepper> ts) { _time_stepper = ts; }

  /**
   * Get the timestepper.
   */
  virtual std::string getTimeStepperName() override;

  /**
   * Get the time scheme used
   * @return MooseEnum with the time scheme
   */
  MooseEnum getTimeScheme() { return _time_scheme; }

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
  Real getStartTime() { return _start_time; }

  /**
   * Get the end time
   * @return The end time
   */
  Real & endTime() { return _end_time; }

  /**
   * Get the timestep tolerance
   * @return The timestep tolerance
   */
  Real & timestepTol() { return _timestep_tolerance; }

  /**
   * Get the verbose output flag
   * @return The verbose output flag
   */
  bool & verbose() { return _verbose; }

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

  void parentOutputPositionChanged() override { _fe_problem.parentOutputPositionChanged(); }

  /**
   * Get the number of Picard iterations performed
   * @return Number of Picard iterations performed
   */
  // Because this returns the number of Picard iterations, rather than the current
  // iteration count (which starts at 0), increment by 1.
  Real numPicardIts() { return _picard_it + 1; }

protected:
  /**
   * This should execute the solve for one timestep.
   */
  virtual void solveStep(Real input_dt = -1.0);

  /// Here for backward compatibility
  FEProblemBase & _problem;

  MooseEnum _time_scheme;
  std::shared_ptr<TimeStepper> _time_stepper;

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

  /// Is it our first time through the execution loop?
  bool & _first;

  /// Whether or not the multiapps failed during the last timestem
  bool & _multiapps_converged;

  /// Whether or not the last solve converged
  bool & _last_solve_converged;

  /// Whether step should be repeated due to xfem modifying the mesh
  bool _xfem_repeat_step;
  unsigned int _xfem_update_count;
  unsigned int _max_xfem_update;

  Real _end_time;
  Real _dtmin;
  Real _dtmax;
  unsigned int _num_steps;
  int _n_startup_steps;
  unsigned int _steps_taken;

  /**
   * Steady state detection variables:
   */
  bool _trans_ss_check;
  Real _ss_check_tol;
  Real _ss_tmin;
  Real & _sln_diff_norm;
  Real & _old_time_solution_norm;

  std::set<Real> & _sync_times;

  bool _abort;

  ///if to use time interval output
  bool & _time_interval;
  Real _next_interval_output_time;
  Real _time_interval_output_interval;

  Real _start_time;
  Real _timestep_tolerance;
  Real & _target_time;
  bool _use_multiapp_dt;

  /**
   * Picard Related
   */
  /// Number of Picard iterations to perform
  int & _picard_it;
  Real _picard_max_its;
  bool & _picard_converged;
  Real & _picard_initial_norm;
  Real & _picard_timestep_begin_norm;
  Real & _picard_timestep_end_norm;
  Real _picard_rel_tol;
  Real _picard_abs_tol;

  ///should detailed diagnostic output be printed
  bool _verbose;

  Real _solution_change_norm;

  void setupTimeIntegrator();
};

#endif // TRANSIENTEXECUTIONER_H
