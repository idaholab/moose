//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSurrogateAction.h"
#include "Factory.h"
#include "FEProblem.h"

registerMooseAction("StochasticToolsApp", AddSurrogateAction, "add_surrogate");

defineLegacyParams(AddSurrogateAction);

InputParameters
AddSurrogateAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Adds SurrogateModel objects contained within the `[Surrogates]` input block.");
  return params;
}

AddSurrogateAction::AddSurrogateAction(InputParameters params) : MooseObjectAction(params) {}

void
AddSurrogateAction::act()
{
  _problem->addUserObject(_type, _name, _moose_object_pars);
}
