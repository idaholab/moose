//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CompositionDT.h"
#include "MooseApp.h"

#include <numeric>

registerMooseObject("MooseApp", CompositionDT);

InputParameters
CompositionDT::validParams()
{
  InputParameters params = TimeStepper::validParams();

  params.addRequiredParam<std::vector<std::string>>(
      "inputs", "The input TimeSteppers.  This can either be N generators or 1 generator.");
  params.addRequiredParam<MooseEnum>(
      "composition_type",
      getCompositionTypes(),
      "Provide a compose method to operate on TimeSteppers. Avaliable methods includes max step, "
      "min step, average of steppers' step and limiting");
  params.addParam<Real>("dt", "Initial value of dt");

  params.addClassDescription("Compose multiple TimeSteppers together to generate time step size.");

  return params;
}

CompositionDT::CompositionDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _composition_type(getParam<MooseEnum>("composition_type")),
    _inputs(getParam<std::vector<std::string>>("inputs")),
    _has_initial_dt(isParamValid("dt")),
    _initial_dt(_has_initial_dt ? getParam<Real>("dt") : 0.)
{
}

Real
CompositionDT::computeInitialDT()
{
  if (_has_initial_dt)
    return _initial_dt;
  else
    return computeDT();
}

Real
CompositionDT::computeDT()
{
  for (const auto & name : _inputs)
  {
    auto time_stepper = _app.getTimeStepperSystem().getTimeStepper(name);
    time_stepper->computeStep();
    Real dt = time_stepper->getCurrentDT();
    std::cout << "name: " << name << " dt: " << dt << std::endl;
    _dts[name] = dt;
  }

  _dt = produceCompositionDT();
  std::cout << "current dt: " << _dt << std::endl;
  return _dt;
}

Real
CompositionDT::produceCompositionDT()
{
  switch (_composition_type)
  {
    case 0: /*max*/
      return maxTimeStep();
    case 1: /*min*/
      return minTimeStep();
    case 2: /*average*/
      return averageTimeStep();
    case 3: /*limiting*/
      return limitingTimeStep();
    default:
      mooseError("CompositionDT: the supplied composition type is not in the list. Available "
                 "options are: ",
                 getCompositionTypes().getRawNames());
  }
}

Real
CompositionDT::maxTimeStep()
{
  auto maxDT = std::max_element(
      _dts.begin(), _dts.end(), [](const auto & x, const auto & y) { return x.second < y.second; });

  return maxDT->second;
}

Real
CompositionDT::minTimeStep()
{
  auto minDT = std::min_element(
      _dts.begin(), _dts.end(), [](const auto & a, const auto & b) { return a.second < b.second; });
  return minDT->second;
}

Real
CompositionDT::averageTimeStep()
{

  Real total_sum =
      std::accumulate(_dts.begin(),
                      _dts.end(),
                      0.0,
                      [](auto prev_sum, auto & entry) { return prev_sum + entry.second; });

  return total_sum / static_cast<float>(_dts.size());
}

Real
CompositionDT::limitingTimeStep()
{
  return averageTimeStep();
}
