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
    _growth_factor(getParam<Real>("growth_factor")),
    _last_dt(declareRestartableData<Real>("last_dt", 0))
{
}

Stepper*
ConstantDT::buildStepper()
{
  Stepper * inner = StepperIf::converged(new MultStepper(_growth_factor, new ReturnPtrStepper(&_last_dt)), new MultStepper(0.5, new ReturnPtrStepper(&_last_dt)));
  inner = new MinOfStepper(new ConstStepper(_constant_dt), inner, 0);
  inner = StepperIf::initialN(new ConstStepper(_constant_dt), inner, _executioner.n_startup_steps());
  inner = StepperIf::converged(inner, new MultStepper(0.5));
  InstrumentedStepper* s = new InstrumentedStepper(&_last_dt);
  s->setStepper(inner);
  return s;
}
