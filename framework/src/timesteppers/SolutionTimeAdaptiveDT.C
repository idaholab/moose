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

#include "SolutionTimeAdaptiveDT.h"
#include "FEProblem.h"
#include "Transient.h"

template<>
InputParameters validParams<SolutionTimeAdaptiveDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<Real>("percent_change", 0.1, "Percentage to change the timestep by.  Should be between 0 and 1");
  params.addParam<int>("initial_direction", 1, "Direction for the first step.  1 for up... -1 for down. ");
  params.addRequiredParam<Real>("dt", "The timestep size between solves");
  return params;
}

SolutionTimeAdaptiveDT::SolutionTimeAdaptiveDT(const InputParameters & parameters) :
    TimeStepper(parameters),
    _direction(getParam<int>("initial_direction")),
    _percent_change(getParam<Real>("percent_change"))
{
}

StepperBlock *
SolutionTimeAdaptiveDT::buildStepper()
{
  StepperBlock * s = new SolveTimeAdaptiveBlock(_direction, _percent_change);
  s = BaseStepper::converged(s, BaseStepper::mult(0.5));
  s = BaseStepper::initialN(BaseStepper::constant(getParam<Real>("dt")), s, _executioner.n_startup_steps());
  return s;
}
