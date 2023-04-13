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

class CompositionDT : public TimeStepper
{
public:
  static InputParameters validParams();

  CompositionDT(const InputParameters & parameters);

  // Find the maximum time step size within all input time stepper(s)
  Real maxTimeStep(const std::map<const std::string, Real> & dts);

  // Find the minimum time step size within all input time stepper(s)
  Real minTimeStep(const std::map<const std::string, Real> & dts);

  /**
   * Find the composed time step size by applying composition rule and compare with the time step
   * size from base time stepper
   * @param dts stores time step size(s) from input time stepper(s)
   * @param basedt time step size from the base time stepper
   */
  Real produceCompositionDT(const Real & max_dt, const Real & min_dt, const Real & basedt);

  // Setup a time stepper with the given name
  std::shared_ptr<TimeStepper> getTimeStepper(const std::string & name);

  // Find the time point to hit at current time step
  Real getSequenceSteppers();

protected:
  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

private:
  // the time step size computed by the Composition TimeStepper
  Real _dt;

  // whether or not has an initial time step size
  bool _has_initial_dt;

  // the initial time step size
  Real _initial_dt;

  // the name of the base time stepper
  const std::string _base_timestepper;

  // the name of the time sequence stepper
  std::vector<std::string> _hit_timestepper_name;

  // the names of input time stepper(s) for maximum dt
  std::vector<std::string> _maximum_step_inputs;

  // the names of input time stepper(s) for minimum dt
  std::vector<std::string> _minimum_step_inputs;
};
