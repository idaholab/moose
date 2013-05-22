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

class TimeStepper;
class FEProblem;
class Transient;

template<>
InputParameters validParams<TimeStepper>();

/**
 * Base class for time stepping
 */
class TimeStepper : public MooseObject
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
   * Take a time step
   */
  virtual void step();

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
   * This returns suggested dt. It may not be the one finally used (Transient executioner has the final word on that)
   * @return The suggested dt to take
   */
  virtual Real computeDT() = 0;

  virtual void forceTimeStep(Real dt);

protected:
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

  /// Whether or not the previous solve converged.
  bool _converged;
  /// Size of the current time step
  Real _current_dt;
};

#endif /* TIMESTEPPER_H */
