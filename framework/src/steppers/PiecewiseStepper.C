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

#include "PiecewiseStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<PiecewiseStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addRequiredParam<std::vector<Real> >("times", "The values of t");
  params.addRequiredParam<std::vector<Real> >("dts",   "The values of dt");
  params.addParam<bool>("interpolate", true, "Whether or not to interpolate DT between times.  This is true by default for historical reasons.");
  params.addParam<bool>("sync_to_times", true, "Whether or not to perfectly land on the given times");

  return params;
}

PiecewiseStepper::PiecewiseStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _times(getParam<std::vector<Real> >("times")),
    _dts(getParam<std::vector<Real> >("dts")),
    _interpolate(getParam<bool>("interpolate")),
    _sync_to_times(getParam<bool>("sync_to_times")),
    _linear_interpolation(_times, _dts)
{
  if (_sync_to_times)
  {
    // Save off the desired output_name
    auto output_name = outputName();

    // Set this output name to something else
    setOutputName(uName("start"));

    auto params = _factory.getValidParams("FixedTimesStepper");

    // Feed _this_ output into the FixedTimesStepper
    params.set<StepperName>("incoming_stepper") = outputName();

    // Copy over the times to hit
    params.set<std::vector<Real> >("times") = _times;

    // Set the output of that Stepper to be the original output of _this_ Stepper
    params.set<StepperName>("_output_name") = output_name;

    // Add the Stepper to the system
    _fe_problem_base.addStepper("FixedTimesStepper", uName("fixed"), params);
  }
}

Real
PiecewiseStepper::computeInitialDT()
{
  return _dts[0];
}

Real
PiecewiseStepper::computeDT()
{
  if (_interpolate)
    return _linear_interpolation.sample(_time);

  mooseAssert(!_times.empty(), "No _times given to PiecewiseStepper");

  if (MooseUtils::relativeFuzzyGreaterEqual(_time, _times.back()))
    return _dts.back();

  for (auto i = beginIndex(_times); i < _times.size() - 1; i++)
    if (MooseUtils::relativeFuzzyLessThan(_time, _times[i + 1]))
      return _dts[i];

  return _dts.back();
}

Real
PiecewiseStepper::computeFailedDT()
{
  return 0.5 * _dt[0]; // _dt[0] was the dt actually used in the last timestep
}
