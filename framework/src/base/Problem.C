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

#include "Problem.h"
#include "Factory.h"
#include "Function.h"
#include "TimePeriod.h"

template<>
InputParameters validParams<Problem>()
{
  InputParameters params;
  params.addPrivateParam<std::string>("built_by_action", "create_problem");
  return params;
}

Problem::Problem(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    _cli_option_found(false),
    _color_output(false)
{
}

Problem::~Problem()
{
  for (unsigned int i=0; i<_time_periods.size(); ++i)
    delete _time_periods[i];
}

TimePeriod &
Problem::addTimePeriod(const std::string & name, Real start_time)
{
  TimePeriod * tp = new TimePeriod(name, start_time);
  _time_periods.push_back(tp);

  return *tp;
}

TimePeriod *
Problem::getTimePeriodByName(const std::string & name)
{
  for (unsigned int i=0; i<_time_periods.size(); ++i)
    if (_time_periods[i]->name() == name)
      return _time_periods[i];
  return NULL;
}

const char *
Problem::setColor(const char * color) const
{
  if (_color_output)
  {
    std::string _color(color);
    if (_cli_option_found && _color.length() == 8 && _color[3] == '2')
      _color.replace(3, 1, "5", 1);
    return _color.c_str();
  }
  return "";
}

const std::vector<TimePeriod *> &
Problem::getTimePeriods() const
{
  return _time_periods;
}

const std::string &
Problem::name()
{
  return _name;
}
