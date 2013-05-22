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

  return params;
}

FunctionDT::FunctionDT(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters),
    _time_t(getParam<std::vector<Real> >("time_t")),
    _time_ipol(_time_t, getParam<std::vector<Real> >("time_dt")),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_occurred(false)
{
}

FunctionDT::~FunctionDT()
{
}

void
FunctionDT::init()
{
  // insert times as sync points except the very first one
  _executioner.syncTimes().insert(_time_t.begin() + 1, _time_t.end());
}

Real
FunctionDT::computeDT()
{
  _current_dt = _time_ipol.sample(_time);
  if (_cutback_occurred && (_current_dt > _dt * _growth_factor))
    _current_dt = _dt * _growth_factor;
  _cutback_occurred = false;

  return _current_dt;
}

void
FunctionDT::rejectStep()
{
  _cutback_occurred = true;
  _fe_problem.restoreSolutions();
}
