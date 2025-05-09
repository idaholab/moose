//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSurrogateAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "SurrogateModel.h"

registerMooseAction("StochasticToolsApp", AddSurrogateAction, "add_trainer");
registerMooseAction("StochasticToolsApp", AddSurrogateAction, "add_surrogate");

InputParameters
AddSurrogateAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds SurrogateTrainer and SurrogateModel objects contained within "
                             "the `[Trainers]` and `[Surrogates]` input blocks.");
  return params;
}

AddSurrogateAction::AddSurrogateAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddSurrogateAction::act()
{
  if (_current_task == "add_trainer")
    _problem->addUserObject(_type, _name, _moose_object_pars);
  else if (_current_task == "add_surrogate")
    _problem->addObject<SurrogateModel>(_type, _name, _moose_object_pars, /* threaded = */ false);
}
