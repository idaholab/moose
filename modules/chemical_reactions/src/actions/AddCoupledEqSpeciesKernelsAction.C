/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddCoupledEqSpeciesKernelsAction.h"
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
validParams<AddCoupledEqSpeciesKernelsAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "primary_species", "The list of primary variables to add");
  params.addParam<std::string>("reactions", "The list of aqueous equilibrium reactions");
  params.addParam<std::string>("pressure", "Checks if pressure is a primary variable");
  return params;
}

AddCoupledEqSpeciesKernelsAction::AddCoupledEqSpeciesKernelsAction(const InputParameters & params)
  : Action(params)
{
}

void
AddCoupledEqSpeciesKernelsAction::act()
{
  // Reading primary species and reaction network from the input file
  std::vector<NonlinearVariableName> vars =
      getParam<std::vector<NonlinearVariableName>>("primary_species");
  std::string reactions = getParam<std::string>("reactions");

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

  pcrecpp::StringPiece input(reactions);

  pcrecpp::StringPiece single_reaction, term;
  Real equal_coeff;

  std::vector<std::vector<bool>> primary_participation(vars.size());
  std::vector<string> eq_species;
  std::vector<Real> weight;
  std::vector<Real> eq_const;
  std::vector<std::vector<Real>> sto_u(vars.size());
  std::vector<std::vector<std::vector<Real>>> sto_v(vars.size());
  std::vector<std::vector<std::vector<VariableName>>> coupled_v(vars.size());

  std::vector<std::vector<Real>> stos;
  std::vector<std::vector<std::string>> primary_species_involved;

  unsigned int n_reactions = 0;

  // Start parsing
  // Going into every single reaction
  std::ostringstream oss;

  while (re_reactions.FindAndConsume(&input, &single_reaction, &equal_coeff))
  {
    n_reactions += 1;
    oss << "\n\n" << n_reactions << "_th reaction: " << single_reaction << std::endl;

    eq_const.push_back(equal_coeff);
    oss << "\nEquilibrium: " << eq_const[n_reactions - 1] << std::endl;

    // capture all of the terms
    std::string species, coeff_str;
    Real coeff;
    int sign = 1;
    bool secondary = false;

    std::vector<Real> local_stos;
    std::vector<std::string> local_species_list;

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
          oss << "\nSpecies: " << species << "\n"
              << "Coeff: " << coeff << std::endl;
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
        oss << "Operator: " << term << "\n\n";

        if (term == "=")
          secondary = true;
      }
      else
        mooseError("Error parsing term: ", term.as_string());
    }

    oss << "\nEquilibrium Species: " << eq_species[n_reactions - 1] << std::endl;

    stos.push_back(local_stos);
    primary_species_involved.push_back(local_species_list);
  }

  if (n_reactions == 0)
    mooseError("No equilibrium reaction provided!");
  // End parsing

  oss << "Number of reactions: " << n_reactions << std::endl;

  // Start picking out primary species and coupled primary species and assigning corresponding
  // stoichiomentric coefficients
  for (unsigned int i = 0; i < vars.size(); ++i)
  {
    oss << "\nPrimary species - " << vars[i] << std::endl;

    sto_u[i].resize(n_reactions);
    sto_v[i].resize(n_reactions);
    coupled_v[i].resize(n_reactions);
    weight.resize(n_reactions);

    primary_participation[i].resize(n_reactions, false);
    for (unsigned int j = 0; j < n_reactions; ++j)
    {
      for (unsigned int k = 0; k < primary_species_involved[j].size(); ++k)
        if (primary_species_involved[j][k] == vars[i])
          primary_participation[i][j] = true;

      oss << "\nPrimary species " << vars[i] << " participation in " << j
          << "_th reaction (0 or 1): " << primary_participation[i][j] << std::endl;

      if (primary_participation[i][j])
      {
        for (unsigned int k = 0; k < primary_species_involved[j].size(); ++k)
        {
          if (primary_species_involved[j][k] == vars[i])
          {
            sto_u[i][j] = stos[j][k];
            weight[j] = stos[j][k];
            oss << "\nEq weight: " << weight[j] << std::endl;
          }
          else
          {
            sto_v[i][j].push_back(stos[j][k]);
            coupled_v[i][j].push_back(primary_species_involved[j][k]);
          }
        }

        oss << "\n#Coupled species: " << coupled_v[i][j].size() << std::endl;
        oss << "\nCoupled species: ";

        for (unsigned int m = 0; m < coupled_v[i][j].size(); ++m)
          oss << coupled_v[i][j][m] << "  " << std::endl;
      }
    }
  }

  // Done parsing, adding kernels
  for (unsigned int i = 0; i < vars.size(); ++i)
  {
    //  Adding the coupled kernels if the primary species participates in this equilibrium reaction
    for (unsigned int j = 0; j < eq_const.size(); ++j)
    {
      if (primary_participation[i][j])
      {
        // Building kernels for equilbirium aqueous species
        InputParameters params_sub = _factory.getValidParams("CoupledBEEquilibriumSub");
        params_sub.set<NonlinearVariableName>("variable") = vars[i];
        params_sub.set<Real>("weight") = weight[j];
        params_sub.set<Real>("log_k") = eq_const[j];
        params_sub.set<Real>("sto_u") = sto_u[i][j];
        params_sub.set<std::vector<Real>>("sto_v") = sto_v[i][j];
        params_sub.set<std::vector<VariableName>>("v") = coupled_v[i][j];
        _problem->addKernel(
            "CoupledBEEquilibriumSub", vars[i] + "_" + eq_species[j] + "_sub", params_sub);

        oss << vars[i] + "_" + eq_species[j] + "_sub"
            << "\n";

        InputParameters params_cd = _factory.getValidParams("CoupledDiffusionReactionSub");
        params_cd.set<NonlinearVariableName>("variable") = vars[i];
        params_cd.set<Real>("weight") = weight[j];
        params_cd.set<Real>("log_k") = eq_const[j];
        params_cd.set<Real>("sto_u") = sto_u[i][j];
        params_cd.set<std::vector<Real>>("sto_v") = sto_v[i][j];
        params_cd.set<std::vector<VariableName>>("v") = coupled_v[i][j];
        _problem->addKernel(
            "CoupledDiffusionReactionSub", vars[i] + "_" + eq_species[j] + "_cd", params_cd);

        oss << vars[i] + "_" + eq_species[j] + "_diff"
            << "\n";

        oss << "whether pressure is present" << _pars.isParamValid("pressure") << "\n";

        if (_pars.isParamValid("pressure"))
        {
          std::string p;
          p = getParam<std::string>("pressure");
          std::vector<VariableName> press = {p};
          oss << "coupled gradient of p" << press[0] << "\n";

          InputParameters params_conv = _factory.getValidParams("CoupledConvectionReactionSub");
          params_conv.set<NonlinearVariableName>("variable") = vars[i];
          params_conv.set<Real>("weight") = weight[j];
          params_conv.set<Real>("log_k") = eq_const[j];
          params_conv.set<Real>("sto_u") = sto_u[i][j];
          params_conv.set<std::vector<Real>>("sto_v") = sto_v[i][j];
          params_conv.set<std::vector<VariableName>>("v") = coupled_v[i][j];
          // Pressure is required to be named as "pressure" if it is a primary variable
          params_conv.set<std::vector<VariableName>>("p") = press;
          _problem->addKernel(
              "CoupledConvectionReactionSub", vars[i] + "_" + eq_species[j] + "_conv", params_conv);

          oss << vars[i] + "_" + eq_species[j] + "_conv"
              << "\n";
        }
      }
    }
  }
  oss << "\n";
  _console << oss.str();
}
