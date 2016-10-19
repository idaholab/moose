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

#include "ConstantDT.h"
#include "Stepper.h"
#include "Transient.h"

template<>
InputParameters validParams<ConstantDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<Real>("dt", "Size of the time step");
  params.addRangeCheckedParam<Real>("growth_factor", 2, "growth_factor>=1",
    "Maximum ratio of new to previous timestep sizes following a step that required the time"
    " step to be cut due to a failed solve.");
  return params;
}

ConstantDT::ConstantDT(const InputParameters & parameters) :
    TimeStepper(parameters),
    _constant_dt(getParam<Real>("dt")),
    _growth_factor(getParam<Real>("growth_factor"))
{
}

Stepper*
ConstantDT::buildStepper()
{
  int n_startup_steps = _executioner.n_startup_steps();

  InstrumentedStepper* s = new InstrumentedStepper();
  *s->dtPtr() = getCurrentDT(); // required for restart

  Stepper* inner = new GrowShrinkStepper(0.5, _growth_factor, new ReturnPtrStepper(s->dtPtr()));
  inner = new MinOfStepper(new ConstStepper(_constant_dt), inner, 0);
  inner = new StartupStepper(inner, _constant_dt, n_startup_steps);

  s->setStepper(inner);
  return s;
}

