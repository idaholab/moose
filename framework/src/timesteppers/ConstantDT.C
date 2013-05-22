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
#include "FEProblem.h"
#include "Transient.h"

template<>
InputParameters validParams<ConstantDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<Real>("dt", "Size of the time step");
  return params;
}

ConstantDT::ConstantDT(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters)
{
  _current_dt = getParam<Real>("dt");
}

ConstantDT::~ConstantDT()
{
}

Real
ConstantDT::computeDT()
{
  return _current_dt;
}

void
ConstantDT::rejectStep()
{
  if (_current_dt <= _dt_min)
    mooseError("Solve failed and timestep already at or below dtmin, cannot continue!");
  // cut the time step in a half
  if (0.5 * _current_dt >= _dt_min)
    _current_dt = 0.5 * _current_dt;
  else // (0.5 * _current_dt < _dt_min)
    _current_dt = _dt_min;

  _fe_problem.restoreSolutions();
}
