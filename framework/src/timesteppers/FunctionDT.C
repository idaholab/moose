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

template <>
InputParameters
validParams<FunctionDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<std::vector<Real>>("time_t", "The values of t");
  params.addRequiredParam<std::vector<Real>>("time_dt", "The values of dt");
  params.addParam<Real>("growth_factor",
                        std::numeric_limits<Real>::max(),
                        "Maximum ratio of new to previous timestep sizes following a step that "
                        "required the time step to be cut due to a failed solve.");
  params.addParam<Real>("min_dt", 0, "The minimal dt to take.");
  params.addParam<bool>("interpolate",
                        true,
                        "Whether or not to interpolate DT between times.  "
                        "This is true by default for historical reasons.");

  return params;
}

FunctionDT::FunctionDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _time_t(getParam<std::vector<Real>>("time_t")),
    _time_dt(getParam<std::vector<Real>>("time_dt")),
    _time_ipol(_time_t, getParam<std::vector<Real>>("time_dt")),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_occurred(false),
    _min_dt(getParam<Real>("min_dt")),
    _interpolate(getParam<bool>("interpolate"))
{
  _time_knots = _time_t;
}

void
FunctionDT::init()
{
}

void
FunctionDT::removeOldKnots()
{
  while ((_time_knots.size() > 0) &&
         (*_time_knots.begin() <= _time || std::abs(*_time_knots.begin() - _time) < 1e-10))
    _time_knots.erase(_time_knots.begin());
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
  Real local_dt = 0;

  if (_interpolate)
    local_dt = _time_ipol.sample(_time);
  else // Find where we are
  {
    unsigned int i = 0;
    if (MooseUtils::relativeFuzzyGreaterEqual(_time, _time_t.back()))
    {
      i = _time_t.size();
    }
    else
    {
      for (; i < _time_t.size() - 1; i++)
        if (MooseUtils::relativeFuzzyLessThan(_time, _time_t[i + 1]))
          break;
    }

    // Use the last dt after the end
    if (i == _time_t.size())
      local_dt = _time_dt.back();
    else
      local_dt = _time_dt[i];
  }

  // sync to time knot
  if ((_time_knots.size() > 0) && (_time + local_dt >= (*_time_knots.begin())))
    local_dt = (*_time_knots.begin()) - _time;
  // honor minimal dt
  if (local_dt < _min_dt)
    local_dt = _min_dt;

  if (_cutback_occurred && (local_dt > _dt * _growth_factor))
    local_dt = _dt * _growth_factor;
  _cutback_occurred = false;

  return local_dt;
}

void
FunctionDT::postStep()
{
  removeOldKnots();
}

void
FunctionDT::rejectStep()
{
  _cutback_occurred = true;
  TimeStepper::rejectStep();
}
