//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionDT.h"
#include "Function.h"
#include "PiecewiseBase.h"
#include <limits>

registerMooseObject("MooseApp", FunctionDT);

InputParameters
FunctionDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  // TODO: This will be required when time_t and time_dt is removed
  params.addParam<FunctionName>(
      "function", "The name of the time-dependent function that prescribes the time step size.");
  params.addParam<std::vector<Real>>("time_t", "The values of t");
  params.addParam<std::vector<Real>>("time_dt", "The values of dt");
  params.addParam<Real>("growth_factor",
                        std::numeric_limits<Real>::max(),
                        "Maximum ratio of new to previous timestep sizes.");
  params.addParam<Real>("min_dt", 0, "The minimal dt to take.");
  // TODO: this can be removed when time_t and time_dt is removed
  params.addParam<bool>("interpolate",
                        true,
                        "Whether or not to interpolate DT between times.  "
                        "This is true by default for historical reasons.");
  params.addClassDescription(
      "Timestepper whose steps vary over time according to a user-defined function");

  return params;
}

FunctionDT::FunctionDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    FunctionInterface(this),
    _time_t(getParam<std::vector<Real>>("time_t")),
    _time_dt(getParam<std::vector<Real>>("time_dt")),
    _function(nullptr),
    _growth_factor(getParam<Real>("growth_factor")),
    _min_dt(getParam<Real>("min_dt")),
    _interpolate(getParam<bool>("interpolate"))
{
  // TODO: remove this when `time_t` and `time_dt` is removed
  if ((isParamValid("time_t") && isParamValid("time_dt")) && !isParamValid("function"))
    mooseDeprecated(name(),
                    ": Using `time_t` and `time_dt` parameter is deprecated. Switch your input "
                    "file to using `function` parameter.\n",
                    "  1. Build a new function. If `interpolate` parameter is true use type = "
                    "PiecewiseLinear. If it was false, use PiecewiseConstant.\n",
                    "  2. Copy `time_t` parameter into your function and rename it to `x`.\n",
                    "  3. Copy `time_dt` parameter into your function and rename it to `y`.\n",
                    "  4. Use the `function` parameter in your time stepper and pass your new "
                    "function name into it.\n");
  else if ((isParamValid("time_t") && isParamValid("time_dt")) && isParamValid("function"))
    mooseError(name(),
               ": Using `time_t`, `_time_dt` and `function` at the same time. Use only `function`, "
               "`time_t` and _time_dt is deprecated.");
  else if (!isParamValid("function"))
    mooseError(name(),
               ": Please, specify a function (using the `function` parameter) that will prescribe "
               "the time step size.");

  if (isParamValid("time_t") && isParamValid("time_dt"))
  {
    try
    {
      _time_ipol = std::make_unique<LinearInterpolation>(_time_t, _time_dt);
    }
    catch (std::domain_error & e)
    {
      mooseError("In FunctionDT ", _name, ": ", e.what());
    }

    _time_knots = _time_t;
    _use_function = false;
  }
  else
  {
    _function = &getFunction("function");
    // If dt is given by piece-wise linear and constant function, we add the domain into
    // _time_knots, so that the time stepper hits those time points
    const PiecewiseBase * pw = dynamic_cast<const PiecewiseBase *>(_function);
    if (pw)
    {
      unsigned int n_knots = pw->functionSize();
      for (unsigned int i = 0; i < n_knots; i++)
        _time_knots.push_back(pw->domain(i));
    }

    _use_function = true;
  }
}

void
FunctionDT::init()
{
  removeOldKnots();
}

void
FunctionDT::removeOldKnots()
{
  while ((_time_knots.size() > 0) &&
         (*_time_knots.begin() <= _time || std::abs(*_time_knots.begin() - _time) < 1e-10))
    _time_knots.erase(_time_knots.begin());
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

  if (_use_function)
    local_dt = _function->value(_time, _point_zero);
  else
  {
    if (_interpolate)
      local_dt = _time_ipol->sample(_time);
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
  }

  // sync to time knot
  if ((_time_knots.size() > 0) && (_time + local_dt >= (*_time_knots.begin())))
    local_dt = (*_time_knots.begin()) - _time;
  // honor minimal dt
  if (local_dt < _min_dt)
    local_dt = _min_dt;

  if ((local_dt > (_dt * _growth_factor)) && _dt > 0)
    local_dt = _dt * _growth_factor;

  return local_dt;
}

void
FunctionDT::postStep()
{
  removeOldKnots();
}
