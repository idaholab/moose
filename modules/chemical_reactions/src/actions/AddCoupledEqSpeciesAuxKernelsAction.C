/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddCoupledEqSpeciesAuxKernelsAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"

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

// Regular expression includes
#include "pcrecpp.h"

template <>
InputParameters
validParams<AddCoupledEqSpeciesAuxKernelsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("reactions", "The list of aqueous equilibrium reactions");
  params.addParam<std::vector<std::string>>(
      "secondary_species", "The list of aqueous equilibrium species to be output as aux variables");
  return params;
}

AddCoupledEqSpeciesAuxKernelsAction::AddCoupledEqSpeciesAuxKernelsAction(
    const InputParameters & params)
  : Action(params),
    _reactions(getParam<std::string>("reactions")),
    _secondary_species(getParam<std::vector<std::string>>("secondary_species"))
{
}

void
AddCoupledEqSpeciesAuxKernelsAction::act()
{
  if (_pars.isParamValid("secondary_species"))
  {
    std::set<std::string> aux_species;

    for (unsigned int k = 0; k < _secondary_species.size(); ++k)
      aux_species.insert(_secondary_species[k]);

    // Getting ready for the parsing system
    pcrecpp::RE re_reactions(
        "(.*?)" // the reaction network (any character until the equilibrium coefficient appears)
        "\\s"   // word boundary
        "("     // start capture
        "-?"    // optional minus sign
        "\\d+(?:\\.\\d*)?" // digits followed by optional decimal and more 0 or more digits
        ")"
        "\\b"        // word boundary
        "(?:\\s+|$)" // eat the whitespace
        ,
        pcrecpp::RE_Options().set_extended(true));

    pcrecpp::RE re_terms("(\\S+)");
    pcrecpp::RE re_coeff_and_species("(?: \\(? (.*?) \\)? )" // match the leading coefficent
                                     "([A-Za-z].*)"          // match the species
                                     ,
                                     pcrecpp::RE_Options().set_extended(true));

    pcrecpp::StringPiece input(_reactions);
    pcrecpp::StringPiece single_reaction, term;
    Real equal_coeff;

    std::vector<string> eq_species;
    std::vector<Real> eq_const;
    std::vector<std::vector<Real>> stos;
    std::vector<std::vector<VariableName>> primary_species_involved;

    unsigned int n_reactions = 0;

    // Start parsing
    // Going into every single reaction
    while (re_reactions.FindAndConsume(&input, &single_reaction, &equal_coeff))
    {
      n_reactions += 1;

      eq_const.push_back(equal_coeff);

      // capture all of the terms
      std::string species, coeff_str;
      Real coeff;
      int sign = 1;
      bool secondary = false;

      std::vector<Real> local_stos;
      std::vector<VariableName> local_species_list;

      // Going to find every single term in this reaction, sto_species combos and operators
      while (re_terms.FindAndConsume(&single_reaction, &term))
      {
        // Separating the sto from species
        if (re_coeff_and_species.PartialMatch(term, &coeff_str, &species))
        {
          if (coeff_str.length())
          {
            std::istringstream iss(coeff_str);
            iss >> coeff;
          }
          else
            coeff = 1.0;

          coeff *= sign;

          if (secondary)
            eq_species.push_back(species);
          else
          {
            local_stos.push_back(coeff);
            local_species_list.push_back(species);
          }
        }
        // Finding the operators and assign value of -1.0 to "-" sign
        else if (term == "+" || term == "=" || term == "-")
        {
          if (term == "-")
          {
            sign = -1;
            term = "+";
          }
          else
            sign = 1;

          if (term == "=")
            secondary = true;
        }
        else
          mooseError("Error parsing term: ", term.as_string());
      }

      stos.push_back(local_stos);
      primary_species_involved.push_back(local_species_list);
    }

    if (n_reactions == 0)
      mooseError("No equilibrium reaction provided!");
    // End parsing

    for (unsigned int j = 0; j < n_reactions; ++j)
    {
      // Adding the AqueousEquilibriumRxnAux auxkernel for the list of eq species read in from the
      // input file
      if (aux_species.find(eq_species[j]) != aux_species.end())
      {
        InputParameters params_eq = _factory.getValidParams("AqueousEquilibriumRxnAux");
        params_eq.set<AuxVariableName>("variable") = eq_species[j];
        params_eq.set<Real>("log_k") = eq_const[j];
        params_eq.set<std::vector<Real>>("sto_v") = stos[j];
        params_eq.set<std::vector<VariableName>>("v") = primary_species_involved[j];
        _problem->addAuxKernel("AqueousEquilibriumRxnAux", "aux_" + eq_species[j], params_eq);

        _console << "aux_" + eq_species[j] << "\n";
      }
    }
  }
}
