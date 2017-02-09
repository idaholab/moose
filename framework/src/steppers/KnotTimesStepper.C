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

#include "KnotTimesStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<KnotTimesStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addRequiredParam<StepperName>("incoming_stepper", "The name of the Stepper to get the current dt from");

  params.addRequiredParam<std::vector<Real> >("times", "The values of t");
  params.addRequiredParam<std::vector<Real> >("dts",   "The values of dt");

  return params;
}

KnotTimesStepper::KnotTimesStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _incoming_stepper_dt(getStepperDT("incoming_stepper")),
    _times(getParam<std::vector<Real> >("times")),
    _dts(getParam<std::vector<Real> >("dts"))
{

  // We'll use a FixedTimesStepper to hit the correct times
  auto output_name = outputName();
  setOutputName(uName("start"));

  auto params = _factory.getValidParams("FixedTimesStepper");
  params.set<StepperName>("incoming_stepper") = outputName();
  params.set<std::vector<Real> >("times") = _times;
  params.set<StepperName>("_output_name") = output_name;
  _fe_problem_base.addStepper("FixedTimesStepper", uName("fixed"), params);
}

Real
KnotTimesStepper::computeInitialDT()
{
  return computeDT();
}

Real
KnotTimesStepper::computeDT()
{
  auto tol = _executioner.timestepTol();

  // See if we're near a time knot
  for (auto i = beginIndex(_times); i < _times.size(); i++)
  {
    if (std::abs(_time - _times[i]) < tol)
      return _dts[i];
  }

  // If we're not at a time knot just pass through
  return _incoming_stepper_dt;
}

Real
KnotTimesStepper::computeFailedDT()
{
  return _incoming_stepper_dt;
}
