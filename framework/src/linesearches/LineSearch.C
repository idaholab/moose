//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineSearch.h"
#include "MooseApp.h"
#include "FEProblem.h"

InputParameters
LineSearch::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("LineSearch");
  params.addParam<NonlinearSystemName>(
      "nl_sys", "The nonlinear system this line search should be applied to.");
  return params;
}

LineSearch::LineSearch(const InputParameters & parameters)
  : MooseObject(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblem *>("_fe_problem", "Must be using FEProblem.")),
    _nl_its(0),
    _nl_sys_num(isParamValid("nl_sys")
                    ? _fe_problem.nlSysNum(getParam<NonlinearSystemName>("nl_sys"))
                    : 0),
    _nl(_fe_problem.getNonlinearSystemBase(_nl_sys_num))
{
}
