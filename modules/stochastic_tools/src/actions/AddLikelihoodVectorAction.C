//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddLikelihoodVectorAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "LikelihoodFunctionBaseVector.h"

registerMooseAction("StochasticToolsApp", AddLikelihoodVectorAction, "add_vector_likelihood");

InputParameters
AddLikelihoodVectorAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Adds Likelihood objects allowing multiple measurements per simulation.");
  return params;
}

AddLikelihoodVectorAction::AddLikelihoodVectorAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddLikelihoodVectorAction::act()
{
  _problem->addObject<LikelihoodFunctionBaseVector>(
      _type, _name, _moose_object_pars, /* threaded = */ false);
}
