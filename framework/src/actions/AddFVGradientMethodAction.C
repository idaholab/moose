//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFVGradientMethodAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddFVGradientMethodAction, "add_gradient_method");

InputParameters
AddFVGradientMethodAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an FVGradientMethod object to the simulation.");
  return params;
}

AddFVGradientMethodAction::AddFVGradientMethodAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddFVGradientMethodAction::act()
{
  _problem->addFVGradientMethod(_type, _name, _moose_object_pars);
}
