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

#include "TimePostprocessor.h"
#include "MultiMooseEnum.h"

template<>
InputParameters validParams<TimePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  MooseEnum time_part("integer fractional");
  params.addParam<MooseEnum>("time_part", time_part, "Limits the output to the portion of time to report (interger or fractional), leaving this empty reports the complete time.");
  params.addParam<Real>("fractional_scale", 1e6, "The value that the fractional portion is scaled");
  return params;
}

TimePostprocessor::TimePostprocessor(const std::string & name, InputParameters parameters) :
  GeneralPostprocessor(name, parameters),
  _time_part(getParam<MooseEnum>("time_part")),
  _fractional_scale(getParam<Real>("fractional_scale"))
{
}

PostprocessorValue
TimePostprocessor::getValue()
{
  // The time to report
  Real t = _t + _app.getGlobalTimeOffset();

  // Separate the time into interger and fractional components
  Real int_part;
  Real frac_part;
  frac_part = std::modf(t, &int_part);

  // Report the correct time value
  if (_time_part == "integer")
    return int_part;
  else if (_time_part == "fractional")
    return _fractional_scale * frac_part;
  else
    return t;
}
