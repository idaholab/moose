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

#include "TransientInterface.h"
#include "SubProblem.h"

template<>
InputParameters validParams<TransientInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<std::string> >("time_periods", "Names of time periods this object will be active in, empty means all the time");
  params.addPrivateParam<bool>("implicit", true);
  return params;
}


TransientInterface::TransientInterface(InputParameters & parameters) :
    _ti_subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _is_implicit(parameters.have_parameter<bool>("implicit") ? parameters.get<bool>("implicit") : true),
    _t(_is_implicit ? _ti_subproblem.time() : _ti_subproblem.timeOld()),
    _t_step(_ti_subproblem.timeStep()),
    _dt(_ti_subproblem.dt()),
    _dt_old(_ti_subproblem.dtOld()),
    _time_weight(_ti_subproblem.timeWeights()),
    _is_transient(_ti_subproblem.isTransient())
{
  if (parameters.have_parameter<std::vector<std::string> >("time_periods"))
  {
    const std::vector<std::string> & tp = parameters.get<std::vector<std::string> >("time_periods");
    for (std::vector<std::string>::const_iterator it = tp.begin(); it != tp.end(); ++it)
    {
      TimePeriod * tp = _ti_subproblem.getTimePeriodByName(*it);
      if (tp != NULL)
        _time_periods.push_back(tp);
      else
        mooseWarning("Time period '" + *it + "' does not exists. Typo?");
    }
  }
}

TransientInterface::~TransientInterface()
{
}

bool
TransientInterface::isActive()
{
  // no time period specified -> active all the time
  if (_time_periods.empty())
    return true;

  // look if _t lies in one of our time periods
  for (std::vector<TimePeriod *>::const_iterator it = _time_periods.begin(); it != _time_periods.end(); ++it)
  {
    TimePeriod * period = *it;
    if ((period->_start <= _t) && (_t < period->_end))
      return true;
  }
  return false;
}
