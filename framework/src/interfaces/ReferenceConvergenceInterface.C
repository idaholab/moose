//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReferenceConvergenceInterface.h"
#include "MooseObject.h"

InputParameters
ReferenceConvergenceInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<std::vector<NonlinearVariableName>>>(
      "group_variables",
      "Name of variables that are grouped together to check convergence. (Multiple groups can be "
      "provided, separated by semicolon)");
  return params;
}

ReferenceConvergenceInterface::ReferenceConvergenceInterface(const MooseObject * moose_object)
  : _fi_params(moose_object->parameters()), _use_group_variables(false)
{
  if (_fi_params.isParamValid("group_variables"))
  {
    _group_variables =
        _fi_params.get<std::vector<std::vector<NonlinearVariableName>>>("group_variables");
    _use_group_variables = true;
  }
}

ReferenceConvergenceInterface::~ReferenceConvergenceInterface() {}
