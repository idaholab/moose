//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
    mooseWarning(
        "Function name '",
        _name,
        "' can be interpreted as a parsed function expression. This can be problematic. As an "
        "example you may name a function 'x' whose functional form is 'xy'. You probably wouldn't "
        "do this, but let's assume for the sake of argument. You might also write somewhere in a "
        "consumer object 'function = x'. Well, MOOSE supports direct construction of parsed "
        "functions from 'FunctionName' parameters, so is this consumer going to end up using the "
        "functional form 'x' or 'xy'? It is undefined behavior.");

#ifdef MOOSE_KOKKOS_ENABLED
  if (_moose_object_pars.isParamValid(MooseBase::kokkos_object_param))
    _problem->addKokkosFunction(_type, _name, _moose_object_pars);
  else
#endif
    _problem->addFunction(_type, _name, _moose_object_pars);
}
