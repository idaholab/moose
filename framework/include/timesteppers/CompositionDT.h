//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeStepper.h"
#include "TimeSequenceStepperBase.h"

/**
 * A TimeStepper that takes time steppers as inputs and computes the minimum time step size among
 * all time steppers as output. If any time sequence stepper(s) is supplied for input, CompositionDT
 * will compare the computed minimum time step size with the one needs to hit the time point, then
 * select the smaller value as output. An optional parameter lower_bound is provided
 * to set a lower bound for the computed time step size.
 *
 * The composition rules are listed with priority rank:
 * 1. The time points from time sequence stepper(s) must be hit;
 * 2. The time step size can not go below the lower_bound;
 * 3. Take the minimum value of all input time steppers.
 */
class CompositionDT : public TimeStepper
{
public:
  /**
   * @returns The parameters that are unique to CompositionDT
   *
   * This is separate so that ComposeTimeStepperAction can also
   * add these parameters.
   */
  static InputParameters compositionDTParams();
  static InputParameters validParams();

  CompositionDT(const InputParameters & parameters);

  /**
   * Comparator for sorting by the value of dt for the TimeStepper sets which stored the pairs of
   * the dt and TimeStepper
   */
  struct CompareFirst
  {
    bool operator()(const std::pair<Real, TimeStepper *> & a,
                    const std::pair<Real, TimeStepper *> & b) const
    {
      return a.first < b.first;
    }
  };
  /**
   * Find the composed time step size by selecting the minimum value and compare it
   * with the lower bound if provided
   * @param dts stores time step size(s) from input time stepper(s)
   * @param bound_dts stores time step size(s) from input lower bound time stepper(s)
   */
  Real produceCompositionDT(std::set<std::pair<Real, TimeStepper *>, CompareFirst> & dts,
                            std::set<std::pair<Real, TimeStepper *>, CompareFirst> & bound_dts);

  // Find the time point to hit at current time step
  Real getSequenceSteppersNextTime();

  /**
   * Initialize all the input time stepper(s). Called at the very beginning of
   * Executioner::execute()
   */
  virtual void init() override final;
  virtual void preExecute() override final;
  virtual void preSolve() override final;
  virtual void postSolve() override final;
  virtual void postExecute() override final;
  virtual void preStep() override final;
  virtual void postStep() override final;
  virtual bool constrainStep(Real & dt) override final;

  /**
   * Functions called after the current DT is computed
   */

  /**
   * Take a time step with _current_time_stepper step() function
   */
  virtual void step() override final;

  /**
   * This gets called when time step is accepted for all input time steppers
   */
  virtual void acceptStep() override final;

  /**
   * This gets called when time step is rejected for all input time steppers
   */
  virtual void rejectStep() override final;

  /**
   * The _current_time_stepper is used to check whether convergence was reached on the time step
   */
  virtual bool converged() const override final;

protected:
  virtual Real computeDT() override final;
  virtual Real computeInitialDT() override final;

private:
  template <typename Lambda>
  void actOnTimeSteppers(Lambda && act);

  /**
   * Internal method for querying TheWarehouse for the currently active timesteppers.
   */
  std::vector<TimeStepper *> getTimeSteppers();

  // The time step size computed by the Composition TimeStepper
  Real _dt;

  // Whether or not has an initial time step size
  const bool _has_initial_dt;

  // The initial time step size
  const Real _initial_dt;

  // The time stepper(s) input as lower bound of the time stepper size
  const std::set<std::string> _lower_bound;

  // The time stepper selected for use every time step, based on minimum time step size
  TimeStepper * _current_time_stepper;

  // The time stepper selected every time step as providing the lower bound
  TimeStepper * _largest_bound_time_stepper;

  // The time sequence stepper selected every time step as the one with the closest time to hit
  TimeSequenceStepperBase * _closest_time_sequence_stepper;
};
