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

#include "DynStepper.h"
#include "Stepper.h"
#include "Transient.h"

template<>
InputParameters validParams<DynStepper>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<Real>("dt", "Size of the time step");
  params.addRangeCheckedParam<Real>("growth_factor", 2, "growth_factor>=1",
    "Maximum ratio of new to previous timestep sizes following a step that required the time"
    " step to be cut due to a failed solve.");
  return params;
}

DynStepper::DynStepper(const InputParameters & parameters) :
    TimeStepper(parameters),
    _constant_dt(getParam<Real>("dt")),
    _growth_factor(getParam<Real>("growth_factor")),
    _last_dt(declareRestartableData<Real>("last_dt", 0))
{
}

Stepper* buildStepper(StepperNode n) {
  if (n.val == "[root]") {
    return buildStepper(n.args[0]);
  } else if (n.val != "[list]") {
    throw Err("bad stepper config");
  }

  std::string name = n.args[0].val;
  if (name == "FixedPointStepper") {
    return new FixedPointStepper(n.args[1].getVec<double>(), n.args[2].get<double>());
  } else if (name == "MinOfStepper") {
    return new MinOfStepper(buildStepper(n.args[1]), buildStepper(n.args[2]), n.args[3].get<double>());
  } else if (name == "ConstStepper") {
    return new ConstStepper(n.args[1].get<double>());
  }
  throw Err("unsupported stepper type");
}
Stepper*
DynStepper::buildStepper()
{
  Stepper* inner = new GrowShrinkStepper(0.5, _growth_factor, new ReturnPtrStepper(&_last_dt));
  inner = new MinOfStepper(new ConstStepper(_constant_dt), inner, 0);
  inner = new StartupStepper(inner, _constant_dt, _executioner.n_startup_steps());
  inner = new IfConvergedStepper(inner, new GrowShrinkStepper(0.5, 1.0));

  InstrumentedStepper* s = new InstrumentedStepper(&_last_dt);
  s->setStepper(inner);
  return s;
}
