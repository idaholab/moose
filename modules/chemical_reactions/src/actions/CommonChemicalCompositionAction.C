//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CommonChemicalCompositionAction.h"
#include "ChemicalCompositionAction.h"
#include "ActionWarehouse.h"

registerMooseAction("ChemicalReactionsApp", CommonChemicalCompositionAction, "meta_action");

InputParameters
CommonChemicalCompositionAction::validParams()
{
  InputParameters params = ChemicalCompositionAction::validParams();
  params.addClassDescription("Store common ChemicalComposition action parameters");
  return params;
}

CommonChemicalCompositionAction::CommonChemicalCompositionAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
CommonChemicalCompositionAction::act()
{
  // check if sub-blocks block are found which will use the common parameters
  auto actions = _awh.getActions<ChemicalCompositionAction>();
  if (actions.size() == 0)
    mooseWarning("Common parameters are supplied, but not used in ", parameters().blockLocation());
}
