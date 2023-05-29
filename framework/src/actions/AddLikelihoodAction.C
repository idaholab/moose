//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddLikelihoodAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddLikelihoodAction, "add_likelihood");

InputParameters
AddLikelihoodAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Likelihood object to the simulation.");
  return params;
}

AddLikelihoodAction::AddLikelihoodAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddLikelihoodAction::act()
{
  _problem->addLikelihood(_type, _name, _moose_object_pars);
}
