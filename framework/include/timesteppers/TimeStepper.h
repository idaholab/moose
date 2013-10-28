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

#ifndef TIMESTEPPER_H
#define TIMESTEPPER_H

#include "MooseObject.h"
#include "Restartable.h"

class TimeStepper;
class FEProblem;
class Transient;

template<>
InputParameters validParams<TimeStepper>();

/**
 * Base class for time stepping
 */
class TimeStepper :
  public MooseObject,
  public Restartable
{
public:
  TimeStepper(const std::string & name, InputParameters parameters);
  virtual ~TimeStepper();

  /**
   * Initialize the time stepper. Called at the very beginning of Executioner::execute()
   */
  virtual void init();

  virtual void preExecute() { }
  virtual void preSolve() { }
  virtual void postSolve() { }
  virtual void postExecute() { }

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
   * Apply constraints on dt.
   */
  void constrainStep(Real &dt);

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

  /**
   * If the time step converged
   * @return true if converged, otherwise false
   */
  virtual bool converged();

  /**
   * Get the current_dt
   */
  Real getCurrentDT() { return _current_dt; }

  virtual void forceTimeStep(Real dt);

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

  FEProblem & _fe_problem;
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
  Real & _timestep_tol;

  /// Whether or not the previous solve converged.
  bool _converged;

  /// If true then the next dt will be computed by computeInitialDT()
  bool _reset_dt;

  /// True if dt has been reset
  bool _has_reset_dt;

private:
  /// Size of the current time step as computed by the Stepper.  Note that the actual dt that was taken might be smaller if the Executioner constrained it.
  Real & _current_dt;
};

#endif /* TIMESTEPPER_H */
