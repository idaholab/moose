//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "Restartable.h"
#include "ScalarCoupleable.h"

class FEProblemBase;
class Transient;

/**
 * Base class for time stepping
 */
class TimeStepper : public MooseObject, public Restartable, public ScalarCoupleable
{
public:
  static InputParameters validParams();

  TimeStepper(const InputParameters & parameters);
  virtual ~TimeStepper();

  /**
   * Initialize the time stepper. Called at the very beginning of Executioner::execute()
   */
  virtual void init();

  virtual void preExecute();
  virtual void preSolve() {}
  virtual void postSolve() {}
  virtual void postExecute() {}
  virtual void preStep() {}
  virtual void postStep() {}

  /**
   * Called before a new step is started.
   * This is when the actual computation of the current DT will be done.
   * Because of that this MUST be called only once per step!
   *
   * After calling this function use getCurrentDT() to get the DT
   * that was computed.
   */
  void computeStep();

  /**
   * Called after computeStep() is called.
   * @return true if any type of sync point was hit, false otherwise
   */
  virtual bool constrainStep(Real & dt);

  /**
   * Take a time step
   */
  virtual void step();

  /**
   * This gets called when time step is accepted
   */
  virtual void acceptStep();

  /**
   * This gets called when time step is rejected
   */
  virtual void rejectStep();

  /// Gets the number of failures and returns them.
  unsigned int numFailures() const;

  /**
   * If the time step converged
   * @return true if converged, otherwise false
   */
  virtual bool converged() const;

  /**
   * Get the current_dt
   */
  Real getCurrentDT() { return _current_dt; }

  virtual void forceTimeStep(Real dt);

  /**
   * Set the number of time steps
   * @param num_steps number of time steps
   */
  virtual void forceNumSteps(const unsigned int num_steps);

  ///@{
  /**
   * Add a sync time
   * \todo {Remove after old output system is removed; sync time are handled by OutputWarehouse}
   */
  void addSyncTime(Real sync_time);
  void addSyncTime(const std::set<Real> & times);
  ///@}

protected:
  /**
   * Called to compute _current_dt for the first timestep.
   * Note that this does not return.
   * The TimeStepper's job here is to fill in _current_dt.
   */
  virtual Real computeInitialDT() = 0;

  /**
   * Called to compute _current_dt for a normal step.
   * Note that this does not return.
   * The TimeStepper's job here is to fill in _current_dt.
   */
  virtual Real computeDT() = 0;

  /**
   * Called to compute _current_dt after a solve has failed.
   * Note that this does not return.
   * The TimeStepper's job here is to fill in _current_dt.
   */
  virtual Real computeFailedDT();

  FEProblemBase & _fe_problem;
  /// Reference to transient executioner
  Transient & _executioner;

  /// Values from executioner
  Real & _time;
  Real & _time_old;
  int & _t_step;
  Real & _dt;
  Real & _dt_min;
  Real & _dt_max;
  Real & _end_time;
  std::set<Real> & _sync_times;

  Real & _timestep_tolerance;

  /// whether a detailed diagnostic output should be printed
  const bool & _verbose;

  /// Whether or not the previous solve converged.
  bool _converged;

  /// Cutback factor if a time step fails to converge
  const Real _cutback_factor_at_failure;

  /// If true then the next dt will be computed by computeInitialDT()
  bool _reset_dt;

  /// True if dt has been reset
  bool _has_reset_dt;

  /// Cumulative amount of steps that have failed
  unsigned int _failure_count;

private:
  /// Size of the current time step as computed by the Stepper.  Note that the actual dt that was taken might be smaller if the Executioner constrained it.
  Real & _current_dt;
};
