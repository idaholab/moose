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
  MooseEnum time_part("days hours minutes seconds milliseconds");
  params.addParam<MooseEnum>("time_part", time_part, "Limits the output to the portion of time to report (interger or fractional), leaving this empty reports the complete time.");
  params.addParam<Real>("fractional_scale", 1, "The value that the fractional portion is scaled");
  return params;
}

TimePostprocessor::TimePostprocessor(const std::string & name,  InputParameters parameters) :
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

  // Days
  unsigned int days = t/86400.;
  t -= days*86400.;
  if (_time_part == "days")
    return days;

  // Hours
  unsigned int hours = t/3600.;
  t -= hours*3600;
  if (_time_part == "hours")
    return hours;

  // Minutes
  unsigned int minutes = t/60.;
  t -= minutes*60;
  if (_time_part == "minutes")
    return minutes;

  // Seconds
  unsigned int seconds = t/60.;
  t -= seconds*60;
  if (_time_part == "seconds")
    return minutes;

  // milliseconds
  Real milliseconds = t*1.e6;
  if (_time_part == "milliseconds")
    return milliseconds;

  else
    return  _t + _app.getGlobalTimeOffset();
}
