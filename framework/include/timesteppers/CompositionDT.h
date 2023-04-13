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

  // Find the minimum time step size within all input time stepper(s)
  Real minTimeStep(const std::map<const std::string, Real> & dts);

  /**
   * Find the composed time step size by applying composition rule and compare with the time step
   * size from base time stepper
   * @param dts stores time step size(s) from input time stepper(s)
   * @param basedt time step size from the base time stepper
   */
  Real produceCompositionDT(const std::map<const std::string, Real> & dts);

  // Find the time point to hit at current time step
  Real getSequenceSteppers();

  // Estimate the time step size needed to hit a user specified time
  Real produceHitDT(const Real & composeDT);

protected:
  virtual Real computeDT() override;
  virtual Real computeInitialDT()override;

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
  std::string _hit_timestepper_name;
};
