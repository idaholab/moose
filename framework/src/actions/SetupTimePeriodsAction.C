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

#include "SetupTimePeriodsAction.h"
#include "TimePeriod.h"
#include "FEProblem.h"
#include "Transient.h"

template<>
InputParameters validParams<SetupTimePeriodsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<Real>("start", 0.0, "The start time for this time period");
  params.addParam<std::vector<std::string> >("active_kernels",    "The list of active kernels during this time period (must not be used with \"inactive_kernels\")");
  params.addParam<std::vector<std::string> >("inactive_kernels",  "The list of inactive kernels during this time period (must not be used with \"active_kernels\")");
  params.addParam<std::vector<std::string> >("active_bcs",        "The list of active boundary conditions during this time period (must not be used with \"inactive_bcs\")");
  params.addParam<std::vector<std::string> >("inactive_bcs",      "The list of inactive boundary conditions during this time period (must not be used with \"active_bcs\")");
  return params;
}


SetupTimePeriodsAction::SetupTimePeriodsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
  if (params.isParamValid("active_kernels") && params.isParamValid("inactive_kernels"))
    mooseError(std::string("Either active or inactive kernels may be supplied for time period \"") + name + "\", not both");
  if (params.isParamValid("active_bcs") && params.isParamValid("inactive_bcs"))
    mooseError(std::string("Either active or inactive bcs may be supplied for time period \"") + name + "\", not both");
}

void
SetupTimePeriodsAction::act()
{
  if (_problem != NULL)
  {
    TimePeriod & tp = _problem->addTimePeriod(getShortName(), getParam<Real>("start"));

    if (_pars.isParamValid("active_kernels"))
      tp.addActiveObjects("kernels", getParam<std::vector<std::string> >("active_kernels"));
    else if (_pars.isParamValid("inactive_kernels"))
      tp.addInactiveObjects("kernels", getParam<std::vector<std::string> >("inactive_kernels"));

    if (_pars.isParamValid("active_bcs"))
      tp.addActiveObjects("bcs", getParam<std::vector<std::string> >("active_bcs"));
    else if (_pars.isParamValid("inactive_bcs"))
      tp.addInactiveObjects("bcs", getParam<std::vector<std::string> >("inactive_bcs"));
  }
}
