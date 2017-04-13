/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddSecondarySpeciesAction.h"
#include "MooseUtils.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

template <>
InputParameters
validParams<AddSecondarySpeciesAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<AuxVariableName>>("secondary_species",
                                                "The list of secondary species to add");
  params.addParam<std::vector<std::string>>("kin_reactions", "The list of solid kinetic reactions");
  return params;
}

AddSecondarySpeciesAction::AddSecondarySpeciesAction(const InputParameters & params)
  : Action(params),
    _vars(getParam<std::vector<AuxVariableName>>("secondary_species")),
    _reactions(getParam<std::vector<std::string>>("kin_reactions"))
{
}

void
AddSecondarySpeciesAction::act()
{
  // Checking to see if there are aqueous eqilibrium species to be added as aux variables
  if (_pars.isParamValid("secondary_species"))
  {
    for (unsigned int i = 0; i < _vars.size(); ++i)
    {
      _console << "aux variables: " << _vars[i] << "\t";
      FEType fe_type(Utility::string_to_enum<Order>("first"),
                     Utility::string_to_enum<FEFamily>("lagrange"));
      _problem->addAuxVariable(_vars[i], fe_type);
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
          std::vector<std::string> stos_vars;
          MooseUtils::tokenize(tokens[k], stos_vars, 1, "()");
          if (stos_vars.size() == 1)
          {
            kin_species = stos_vars[0];
            _console << "I'm here and the kin_species is: " << stos_vars[0] << "\n";
          }
        }
        _console << "the " << j + 1 << "-th solid kinetic species: " << kin_species << "\n";
        FEType fe_type(Utility::string_to_enum<Order>("first"),
                       Utility::string_to_enum<FEFamily>("lagrange"));
        _problem->addAuxVariable(kin_species, fe_type);
      }
    }
  }
}
