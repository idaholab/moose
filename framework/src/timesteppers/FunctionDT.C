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

template<>
InputParameters validParams<FunctionDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<std::vector<Real> >("time_t", "The values of t");
  params.addParam<std::vector<Real> >("time_dt", "The values of dt");
  params.addParam<Real>("growth_factor", 2, "Maximum ratio of new to previous timestep sizes following a step that required the time step to be cut due to a failed solve.");
  params.addParam<Real>("min_dt", 0, "The minimal dt to take. Always enforced, even it means missing a sync time or end time.");

  return params;
}

FunctionDT::FunctionDT(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters),
    _time_t(),
    _time_ipol(),
    _growth_factor(getParam<Real>("growth_factor")),
    _min_dt(getParam<Real>("min_dt"))
{
  std::vector<Real> times( getParam< std::vector<Real> >("time_t") );
  _time_ipol.setData( times, getParam< std::vector<Real> >("time_dt") );
  _time_t.insert( times.begin(), times.end() );
}

void
FunctionDT::init()
{
}

void
FunctionDT::removeOldKnots()
{
  while ( ! _time_t.empty() && _time + _timestep_tolerance >= *_time_t.begin() )
    _time_t.erase( _time_t.begin() );
}

void
FunctionDT::preExecute()
{
  TimeStepper::preExecute();
  removeOldKnots();
}

Real
FunctionDT::computeInitialDT()
{
  return computeDT();
}

Real
FunctionDT::computeDT()
{
  Real local_dt = _time_ipol.sample(_time);

  if (local_dt < _min_dt)
    local_dt = _min_dt;

  return local_dt;
}

bool
FunctionDT::constrainStep( Real & dt )
{
  bool at_sync_point = TimeStepper::constrainStep( dt );

  // Adjust to the next sync time if needed
  if ( ! _time_t.empty() && _time + dt + _timestep_tolerance >= *_time_t.begin() )
    dt = *_time_t.begin() - _time;

  if ( dt < _min_dt )
    dt = _min_dt;

  return at_sync_point;
}

void
FunctionDT::acceptStep()
{
  TimeStepper::acceptStep();
  removeOldKnots();
}

void
FunctionDT::rejectStep()
{
  TimeStepper::rejectStep();
}
