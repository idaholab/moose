/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
  params.addParam<std::vector<std::string>>("kin_reactions", "The list of solid kinetic reactions");
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
  : Action(params),
    _secondary_species(getParam<std::vector<AuxVariableName>>("secondary_species")),
    _reactions(getParam<std::vector<std::string>>("kin_reactions"))
{
}

void
AddSecondarySpeciesAction::act()
{
  // Checking to see if there are aqueous eqilibrium species to be added as aux variables
  if (_pars.isParamValid("secondary_species"))
  {
    for (unsigned int i = 0; i < _secondary_species.size(); ++i)
    {
      FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                     Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));

      _problem->addAuxVariable(_secondary_species[i], fe_type);
    }
  }
  else
  {
    // Checking to see if there are solid kinetic species to be added as aux variables
    if (_pars.isParamValid("kin_reactions"))
    {
      for (unsigned int j = 0; j < _reactions.size(); ++j)
      {
        std::vector<std::string> tokens;
        std::string kin_species;

        // Parsing each reaction
        MooseUtils::tokenize(_reactions[j], tokens, 1, "+=");

        for (unsigned int k = 0; k < tokens.size(); ++k)
        {
          std::vector<std::string> stos_secondary_species;
          MooseUtils::tokenize(tokens[k], stos_secondary_species, 1, "()");
          if (stos_secondary_species.size() == 1)
            kin_species = stos_secondary_species[0];
        }

        FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                       Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));

        _problem->addAuxVariable(kin_species, fe_type);
      }
    }
  }
}
