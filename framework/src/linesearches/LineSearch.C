//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineSearch.h"
#include "MooseApp.h"

InputParameters
LineSearch::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("LineSearch");
  return params;
}

LineSearch::LineSearch(const InputParameters & parameters)
  : MooseObject(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblem *>("_fe_problem", "Must be using FEProblem.")),
    _nl_its(0)
{
}
