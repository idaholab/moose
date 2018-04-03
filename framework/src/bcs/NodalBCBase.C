//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalBCBase.h"

template <>
InputParameters
validParams<NodalBCBase>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params += validParams<RandomInterface>();
  params.addParam<std::vector<AuxVariableName>>(
      "save_in",
      "The name of auxiliary variables to save this BC's residual contributions to.  "
      "Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      "The name of auxiliary variables to save this BC's diagonal jacobian "
      "contributions to.  Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)");

  return params;
}

NodalBCBase::NodalBCBase(const InputParameters & parameters)
  : BoundaryCondition(parameters, true), // true is for being Nodal
    RandomInterface(parameters, _fe_problem, _tid, true),
    CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in")),
    _is_eigen(false)
{
}
