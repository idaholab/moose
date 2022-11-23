//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSamplerAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddSamplerAction, "add_sampler");

InputParameters
AddSamplerAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Sampler object to the simulation.");
  return params;
}

AddSamplerAction::AddSamplerAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddSamplerAction::act()
{
  _problem->addSampler(_type, _name, _moose_object_pars);
}
