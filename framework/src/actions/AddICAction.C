//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddICAction.h"
#include "FEProblem.h"
#include "MooseTypes.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<AddICAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddICAction::AddICAction(InputParameters params) : MooseObjectAction(params) {}

void
AddICAction::act()
{
  std::vector<std::string> elements;
  MooseUtils::tokenize<std::string>(_pars.blockFullpath(), elements);

  // The variable name will be the second to last element in the path name
  std::string & var_name = elements[elements.size() - 2];
  _moose_object_pars.set<VariableName>("variable") = var_name;
  _problem->addInitialCondition(_type, var_name, _moose_object_pars);
}
