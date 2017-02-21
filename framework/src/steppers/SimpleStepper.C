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

#include "SimpleStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<SimpleStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addRequiredParam<Real>("dt", "The requested dt.  The dt will be cut when a solve fails and will attempt to regrow back to this value when solves converge.");

  params.addRangeCheckedParam<Real>("growth_factor", 2, "growth_factor>=1", "Maximum ratio of new to previous timestep sizes following a step that required the time step to be cut due to a failed solve.");

  return params;
}

SimpleStepper::SimpleStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _input_dt(getParam<Real>("dt")),
    _growth_factor(getParam<Real>("growth_factor")),
    _prev_dt(declareRestartableData<Real>("prev_dt", _input_dt))
{
}

Real
SimpleStepper::computeInitialDT()
{
  return _prev_dt = _input_dt;
}

Real
SimpleStepper::computeDT()
{
  return _prev_dt = std::min(_input_dt, _growth_factor * _prev_dt);
}

Real
SimpleStepper::computeFailedDT()
{
  return _prev_dt = 0.5 * _dt[0]; // _dt[0] was the dt actually used in the last timestep
}
