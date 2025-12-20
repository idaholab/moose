//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFVInterpolationMethodAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddFVInterpolationMethodAction, "add_interpolation_method");

InputParameters
AddFVInterpolationMethodAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Add an FVInterpolationMethod object to the simulation for later reuse.");
  return params;
}

AddFVInterpolationMethodAction::AddFVInterpolationMethodAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddFVInterpolationMethodAction::act()
{
  _problem->addFVInterpolationMethod(_type, _name, _moose_object_pars);
}
