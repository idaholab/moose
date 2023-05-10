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
 * A TimeStepper takes time steppers as input and compute the minimum time step size among all
 * time steppers as output. If any time sequence stepper(s) is supplied for input, CompositionDT
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
   * Find the composed time step size by selecting the minimum value and compare it
   * with the lower bound if provided
   * @param dts stores time step size(s) from input time stepper(s)
   * @param bound_dts stores time step size(s) from input lower bound time stepper(s)
   */
  Real produceCompositionDT(const std::set<Real> & dts, const std::set<Real> & bound_dts);

  // Find the time point to hit at current time step
  Real getSequenceSteppersNextTime();

protected:
  virtual Real computeDT() override;
  virtual Real computeInitialDT() override;

private:
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
};
