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

#include "FunctionDT.h"
#include "FEProblem.h"
#include "Transient.h"
#include <limits>

template<>
InputParameters validParams<FunctionDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<std::vector<Real> >("time_t", "The values of t");
  params.addRequiredParam<std::vector<Real> >("time_dt", "The values of dt");
  params.addParam<Real>("growth_factor", std::numeric_limits<Real>::max(), "Maximum ratio of new to previous timestep sizes following a step that required the time step to be cut due to a failed solve.");
  params.addParam<Real>("min_dt", 0, "The minimal dt to take.");
  params.addParam<bool>("interpolate", true, "Whether or not to interpolate DT between times.  This is true by default for historical reasons.");

  return params;
}

FunctionDT::FunctionDT(const InputParameters & parameters) :
    TimeStepper(parameters),
    _time_t(getParam<std::vector<Real> >("time_t")),
    _time_dt(getParam<std::vector<Real> >("time_dt")),
    _growth_factor(getParam<Real>("growth_factor")),
    _min_dt(getParam<Real>("min_dt")),
    _interpolate(getParam<bool>("interpolate"))
{
}

Stepper *
FunctionDT::buildStepper()
{
  Stepper* s = new PiecewiseStepper(_time_t, _time_dt, _interpolate);
  s = new MinOfStepper(new FixedPointStepper(_time_t, _executioner.timestepTol()), s, 0);
  s = new DTLimitStepper(s, _min_dt, 1e100, false);
  // this packs a single Stepper* into multiple std::unique_ptr's - bad
  s = new IfConvergedStepper(s, new MinOfStepper(s, new GrowShrinkStepper(0.5, _growth_factor), 0));
  return s;
}
