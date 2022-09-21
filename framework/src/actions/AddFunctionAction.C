//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFunctionAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddFunctionAction, "add_function");

InputParameters
AddFunctionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Function object to the simulation.");
  return params;
}

AddFunctionAction::AddFunctionAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddFunctionAction::act()
{
  FunctionParserBase<Real> fp;
  std::string vars = "x,y,z,t,NaN,pi,e";
  if (fp.Parse(_name, vars) == -1) // -1 for success
    mooseWarning("Function name '" + _name + "' could evaluate as a ParsedFunction");
  _problem->addFunction(_type, _name, _moose_object_pars);
}
