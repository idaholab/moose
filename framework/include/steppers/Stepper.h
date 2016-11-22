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

#ifndef STEPPER_H
#define STEPPER_H

#include "MooseObject.h"
#include "Restartable.h"
#include "StepperInterface.h"
#include "PostprocessorInterface.h"

class Stepper;
class FEProblemBase;
class Factory;
class StepperInfo;
class Transient;

template<>
InputParameters validParams<Stepper>();

/**
 * Base class for time stepping
 */
class Stepper :
  public MooseObject,
  public Restartable,
  public StepperInterface,
  public PostprocessorInterface
{
public:
  Stepper(const InputParameters & parameters);
  virtual ~Stepper();

  /**
   * Compute the timestep size for the the first timestep
   */
  virtual Real computeInitialDT() = 0;

  /**
   * Compute the size of the next timestep
   */
  virtual Real computeDT() = 0;

  /**
   * Compute the size of the next timestep after a convergence failure
   */
  virtual Real computeFailedDT() = 0;

  /**
   * Get the name of the output from this Stepper
   */
  const StepperName & outputName() { return _output_name; }

protected:
  /**
   * Generate a unique name.  Useful for sub-stepper names
   */
  std::string uName(const std::string & suffix) { return name() + "_" + suffix; }

  /**
   * Set the name this Stepper should output as.
   *
   * This is useful for compound steppers.  They can change their own output name
   * and pass their true name in as the output name for the last stepper in the sequence.
   *
   * That way, when other Steppers depend on this one they get the final value from the chain.
   */
  void setOutputName(const StepperName & output_name);

  /// The FEProblem
  FEProblemBase & _fe_problem_base;

  /// Transient Executioner
  Transient & _executioner;

  /// Use this to build sub-Steppers
  Factory & _factory;

  /// The name this Stepper will output as.  This is set from a private param: _output_name
  StepperName _output_name;

private:
  /// Information consumed by Steppers
  StepperInfo & _stepper_info;

protected:
  /**
   * A Stepper can call this to cause MOOSE to backup the system state at the current time.
   * Later, restore() can be called to restore the system at at particular time.
   */
  void backup() { _backup = true; }

  /**
   * Restore the system state at a previous time.
   * That previous time must exactly match the time at which a backup() was made
   *
   * @param restore_time The time to restore the system to
   */
  void restore(Real restore_time) { _restore = true; _restore_time = restore_time; }

  ////// Information from StepperInfo //////
  int & _step_count;
  Real & _time;
  std::deque<Real> & _dt;

  unsigned int & _nonlin_iters;
  unsigned int & _lin_iters;
  std::deque<bool> & _converged;
  std::deque<Real> & _solve_time_secs;

  std::unique_ptr<NumericVector<Number>> & _soln_nonlin;
  std::unique_ptr<NumericVector<Number>> & _soln_aux;
  std::unique_ptr<NumericVector<Number>> & _soln_predicted;

private:
  bool & _backup;
  bool & _restore;
  Real & _restore_time;
};

#endif /* STEPPER_H */
