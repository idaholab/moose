//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddPrimarySpeciesAction.h"
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "Factory.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<AddPrimarySpeciesAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "primary_species", "The list of primary variables to add");
  // Get MooseEnums for the possible order/family options for this variable
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("family",
                             families,
                             "Specifies the family of FE "
                             "shape function to use for the order parameters");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE "
                             "shape function to use for the order parameters");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addClassDescription("Adds Variables for all primary species");
  return params;
}

AddPrimarySpeciesAction::AddPrimarySpeciesAction(const InputParameters & params)
  : Action(params), _vars(getParam<std::vector<NonlinearVariableName>>("primary_species"))
{
}

void
AddPrimarySpeciesAction::act()
{
  for (unsigned int i = 0; i < _vars.size(); ++i)
  {
    FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                   Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));

    _problem->addVariable(_vars[i], fe_type, getParam<Real>("scaling"));
  }
}
