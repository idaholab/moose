/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddCoupledEqSpeciesAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"

#include <sstream>

#include "libmesh/string_to_enum.h"

// Regular expression includes
#include "pcrecpp.h"

template <>
InputParameters
validParams<AddCoupledEqSpeciesAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "primary_species", "The list of primary variables to add");
  params.addParam<std::vector<AuxVariableName>>(
      "secondary_species", "The list of aqueous equilibrium species to be output as aux variables");
  params.addParam<std::string>("reactions", "The list of aqueous equilibrium reactions");
  params.addParam<std::vector<VariableName>>("pressure", "Pressure variable");
  RealVectorValue g(0, 0, 0);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector (default is (0, 0, 0))");
  params.addClassDescription("Adds coupled equilibrium Kernels for primary species");
  return params;
}

AddCoupledEqSpeciesAction::AddCoupledEqSpeciesAction(const InputParameters & params)
  : Action(params),
    _reactions(getParam<std::string>("reactions")),
    _primary_species(getParam<std::vector<NonlinearVariableName>>("primary_species")),
    _secondary_species(getParam<std::vector<AuxVariableName>>("secondary_species")),
    _pressure_var(getParam<std::vector<VariableName>>("pressure")),
    _gravity(getParam<RealVectorValue>("gravity"))
{
}

void
AddCoupledEqSpeciesAction::act()
{
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
  Real eq_coeff;

  std::vector<std::vector<bool>> primary_participation(_primary_species.size());
  std::vector<std::string> eq_species;
  std::vector<Real> weight;
  std::vector<Real> eq_const;
  std::vector<std::vector<Real>> sto_u(_primary_species.size());
  std::vector<std::vector<std::vector<Real>>> sto_v(_primary_species.size());
  std::vector<std::vector<std::vector<VariableName>>> coupled_v(_primary_species.size());

  std::vector<std::vector<Real>> stos;
  std::vector<std::vector<VariableName>> primary_species_involved;

  std::set<std::string> aux_species;
  if (_pars.isParamValid("secondary_species"))
    for (unsigned int k = 0; k < _secondary_species.size(); ++k)
      aux_species.insert(_secondary_species[k]);

  unsigned int n_reactions = 0;
  std::ostringstream oss;
  oss << "Aqueous equilibrium reactions:\n";

  // Start parsing equilibrium reactions
  while (re_reactions.FindAndConsume(&input, &single_reaction, &eq_coeff))
  {
    n_reactions += 1;
    oss << single_reaction << "\n";

    eq_const.push_back(eq_coeff);

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

  // Start picking out primary species and coupled primary species and assigning corresponding
  // stoichiomentric coefficients
  for (unsigned int i = 0; i < _primary_species.size(); ++i)
  {
    sto_u[i].resize(n_reactions);
    sto_v[i].resize(n_reactions);
    coupled_v[i].resize(n_reactions);
    weight.resize(n_reactions);

    primary_participation[i].resize(n_reactions, false);
    for (unsigned int j = 0; j < n_reactions; ++j)
    {
      for (unsigned int k = 0; k < primary_species_involved[j].size(); ++k)
        if (primary_species_involved[j][k] == _primary_species[i])
          primary_participation[i][j] = true;

      if (primary_participation[i][j])
      {
        for (unsigned int k = 0; k < primary_species_involved[j].size(); ++k)
        {
          if (primary_species_involved[j][k] == _primary_species[i])
          {
            sto_u[i][j] = stos[j][k];
            weight[j] = stos[j][k];
          }
          else
          {
            sto_v[i][j].push_back(stos[j][k]);
            coupled_v[i][j].push_back(primary_species_involved[j][k]);
          }
        }
      }
    }
  }

  // Done parsing, adding kernels
  if (_current_task == "add_kernel")
  {
    for (unsigned int i = 0; i < _primary_species.size(); ++i)
    {
      //  Adding the coupled kernels if the primary species participates in this equilibrium
      //  reaction
      for (unsigned int j = 0; j < eq_const.size(); ++j)
      {
        if (primary_participation[i][j])
        {
          // Building kernels for equilbirium aqueous species
          InputParameters params_sub = _factory.getValidParams("CoupledBEEquilibriumSub");
          params_sub.set<NonlinearVariableName>("variable") = _primary_species[i];
          params_sub.set<Real>("weight") = weight[j];
          params_sub.set<Real>("log_k") = eq_const[j];
          params_sub.set<Real>("sto_u") = sto_u[i][j];
          params_sub.set<std::vector<Real>>("sto_v") = sto_v[i][j];
          params_sub.set<std::vector<VariableName>>("v") = coupled_v[i][j];
          _problem->addKernel("CoupledBEEquilibriumSub",
                              _primary_species[i] + "_" + eq_species[j] + "_sub",
                              params_sub);

          InputParameters params_cd = _factory.getValidParams("CoupledDiffusionReactionSub");
          params_cd.set<NonlinearVariableName>("variable") = _primary_species[i];
          params_cd.set<Real>("weight") = weight[j];
          params_cd.set<Real>("log_k") = eq_const[j];
          params_cd.set<Real>("sto_u") = sto_u[i][j];
          params_cd.set<std::vector<Real>>("sto_v") = sto_v[i][j];
          params_cd.set<std::vector<VariableName>>("v") = coupled_v[i][j];
          _problem->addKernel("CoupledDiffusionReactionSub",
                              _primary_species[i] + "_" + eq_species[j] + "_cd",
                              params_cd);

          // If pressure is coupled, add a CoupledConvectionReactionSub Kernel as well
          if (_pars.isParamValid("pressure"))
          {
            InputParameters params_conv = _factory.getValidParams("CoupledConvectionReactionSub");
            params_conv.set<NonlinearVariableName>("variable") = _primary_species[i];
            params_conv.set<Real>("weight") = weight[j];
            params_conv.set<Real>("log_k") = eq_const[j];
            params_conv.set<Real>("sto_u") = sto_u[i][j];
            params_conv.set<std::vector<Real>>("sto_v") = sto_v[i][j];
            params_conv.set<std::vector<VariableName>>("v") = coupled_v[i][j];
            params_conv.set<std::vector<VariableName>>("p") = _pressure_var;
            params_conv.set<RealVectorValue>("gravity") = _gravity;
            _problem->addKernel("CoupledConvectionReactionSub",
                                _primary_species[i] + "_" + eq_species[j] + "_conv",
                                params_conv);
          }
        }
      }
    }
  }

  if (_current_task == "add_aux_kernel")
  {
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
      }
    }
  }

  oss << "\n";
  mooseDoOnce(_console << oss.str());
}
