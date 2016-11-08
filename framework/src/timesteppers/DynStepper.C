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

Stepper*
buildStepper(StepperNode n) {
  if (n.val == "[root]") {
    return buildStepper(n.args[0]);
  } else if (n.val != "[list]") {
    throw Err("bad stepper config");
  }

  std::string name = n.args[0].val;
  if (name == "FixedPointStepper") {
    if (n.args.size() < 3) throw Err("FixedPointStepper needs 2 args");
    return new FixedPointStepper(n.args[1].getVec<double>(), n.args[2].get<double>());
  } else if (name == "MinOfStepper") {
    if (n.args.size() < 4) throw Err("MinOfStepper needs 3 args");
    return new MinOfStepper(buildStepper(n.args[1]), buildStepper(n.args[2]), n.args[3].get<double>());
  } else if (name == "ConstStepper") {
    if (n.args.size() < 2) throw Err("ConstStepper needs 1 args");
    return new ConstStepper(n.args[1].get<double>());
  }
  throw Err("unsupported stepper type");
}

Stepper*
DynStepper::buildStepper()
{
  return nullptr;
}
