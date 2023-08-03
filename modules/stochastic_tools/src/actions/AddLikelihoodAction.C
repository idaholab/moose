//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddLikelihoodAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "LikelihoodFunctionBase.h"

registerMooseAction("StochasticToolsApp", AddLikelihoodAction, "add_likelihood");

InputParameters
AddLikelihoodAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds Likelihood objects.");
  return params;
}

AddLikelihoodAction::AddLikelihoodAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddLikelihoodAction::act()
{
  _problem->addObject<LikelihoodFunctionBase>(
      _type, _name, _moose_object_pars, /* threaded = */ false);
}
