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
#include "TimePeriodOld.h"

template<>
InputParameters validParams<Problem>()
{
  InputParameters params;
  params += validParams<MooseObject>();
  params.registerBase("Problem");
  return params;
}

Problem::Problem(const InputParameters & parameters) :
    MooseObject(parameters),
    _cli_option_found(false),
    _color_output(false),
    _termination_requested(false)
{
}

Problem::~Problem()
{
  for (unsigned int i=0; i<_time_periods.size(); ++i)
    delete _time_periods[i];
}

TimePeriodOld &
Problem::addTimePeriod(const std::string & name, Real start_time)
{
  TimePeriodOld * tp = new TimePeriodOld(name, start_time);
  _time_periods.push_back(tp);

  return *tp;
}

TimePeriodOld *
Problem::getTimePeriodByName(const std::string & name)
{
  for (unsigned int i=0; i<_time_periods.size(); ++i)
    if (_time_periods[i]->name() == name)
      return _time_periods[i];
  return NULL;
}

const std::vector<TimePeriodOld *> &
Problem::getTimePeriods() const
{
  return _time_periods;
}
