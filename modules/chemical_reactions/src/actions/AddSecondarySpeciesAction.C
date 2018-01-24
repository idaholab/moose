//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSecondarySpeciesAction.h"
#include "AddAuxVariableAction.h"
#include "MooseUtils.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<AddSecondarySpeciesAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<AuxVariableName>>("secondary_species",
                                                "The list of secondary species to add");
  // Get MooseEnums for the possible order/family options for this variable
  MooseEnum families(AddAuxVariableAction::getAuxVariableFamilies());
  MooseEnum orders(AddAuxVariableAction::getAuxVariableOrders());
  params.addParam<MooseEnum>("family",
                             families,
                             "Specifies the family of FE "
                             "shape function to use for the order parameters");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE "
                             "shape function to use for the order parameters");
  params.addClassDescription("Adds AuxVariables for all secondary species");
  return params;
}

AddSecondarySpeciesAction::AddSecondarySpeciesAction(const InputParameters & params)
  : Action(params), _secondary_species(getParam<std::vector<AuxVariableName>>("secondary_species"))
{
}

void
AddSecondarySpeciesAction::act()
{
  // Checking to see if there are secondary species to be added as AuxVariables
  if (_pars.isParamValid("secondary_species"))
  {
    for (unsigned int i = 0; i < _secondary_species.size(); ++i)
    {
      FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                     Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));

      _problem->addAuxVariable(_secondary_species[i], fe_type);
    }
  }
}
