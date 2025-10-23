//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddKokkosFunctionAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddKokkosFunctionAction, "add_function");

InputParameters
AddKokkosFunctionAction::validParams()
{
  InputParameters params = KokkosObjectAction::validParams();
  params.addClassDescription("Add a Kokkos Function object to the simulation.");
  return params;
}

AddKokkosFunctionAction::AddKokkosFunctionAction(const InputParameters & params)
  : KokkosObjectAction(params, "Function")
{
}

void
AddKokkosFunctionAction::act()
{
  FunctionParserBase<Real> fp;
  std::string vars = "x,y,z,t,NaN,pi,e";
  if (fp.Parse(_name, vars) == -1) // -1 for success
    mooseWarning("Function name '" + _name + "' could evaluate as a ParsedFunction");
  _problem->addKokkosFunction(_type, _name, _moose_object_pars);
}
