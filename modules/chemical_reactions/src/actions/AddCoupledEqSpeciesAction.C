//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddCoupledEqSpeciesAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"

#include "libmesh/string_to_enum.h"

// Regular expression includes
#include "pcrecpp.h"

registerMooseAction("ChemicalReactionsApp", AddCoupledEqSpeciesAction, "add_kernel");

registerMooseAction("ChemicalReactionsApp", AddCoupledEqSpeciesAction, "add_aux_kernel");

InputParameters
AddCoupledEqSpeciesAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "primary_species", "The list of primary variables to add");
  params.addParam<std::vector<AuxVariableName>>(
      "secondary_species", "The list of aqueous equilibrium species to be output as aux variables");
  params.addParam<std::string>("reactions", "The list of aqueous equilibrium reactions");
  params.addParam<std::vector<VariableName>>("pressure", "Pressure variable");
  RealVectorValue g(0, 0, 0);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector (default is (0, 0, 0))");
  params.addClassDescription("Adds coupled equilibrium Kernels and AuxKernels for primary species");
  return params;
}

AddCoupledEqSpeciesAction::AddCoupledEqSpeciesAction(const InputParameters & params)
  : Action(params),
    _primary_species(getParam<std::vector<NonlinearVariableName>>("primary_species")),
    _secondary_species(getParam<std::vector<AuxVariableName>>("secondary_species")),
    _weights(_primary_species.size()),
    _primary_participation(_primary_species.size()),
    _sto_u(_primary_species.size()),
    _sto_v(_primary_species.size()),
    _coupled_v(_primary_species.size()),
    _input_reactions(getParam<std::string>("reactions")),
    _pressure_var(getParam<std::vector<VariableName>>("pressure")),
    _gravity(getParam<RealVectorValue>("gravity"))
{
  // Parse the aqueous equilibrium reactions
  pcrecpp::RE re_reaction(
      "(.+?)" // single reaction (any character until the equilibrium coefficient appears)
      "\\s"   // word boundary
      "("     // start capture
      "-?"    // optional minus sign
      "\\d+(?:\\.\\d*)?" // digits followed by optional decimal and more digits
      ")"                // end capture
      "\\b"              // word boundary
      "(?:,?\\s*|$)"     // optional comma or end of string
      ,
      pcrecpp::RE_Options().set_extended(true));

  pcrecpp::RE re_terms("(\\S+)");
  pcrecpp::RE re_coeff_and_species("(?: \\(? (.*?) \\)? )" // match the leading coefficent
                                   "([A-Za-z].*)"          // match the species
                                   ,
                                   pcrecpp::RE_Options().set_extended(true));

  pcrecpp::StringPiece input(_input_reactions);

  pcrecpp::StringPiece single_reaction, term;
  std::string single_reaction_str;
  Real eq_coeff;

  // Parse reaction network to extract each individual reaction
  while (re_reaction.FindAndConsume(&input, &single_reaction_str, &eq_coeff))
  {
    _reactions.push_back(single_reaction_str);
    _eq_const.push_back(eq_coeff);
  }

  _num_reactions = _reactions.size();

  if (_num_reactions == 0)
    mooseError("No equilibrium reaction provided!");

  if (_pars.isParamValid("secondary_species"))
    for (unsigned int k = 0; k < _secondary_species.size(); ++k)
      _aux_species.insert(_secondary_species[k]);

  // Start parsing each reaction
  for (unsigned int i = 0; i < _num_reactions; ++i)
  {
    single_reaction = _reactions[i];

    // Capture all of the terms
    std::string species, coeff_str;
    Real coeff;
    int sign = 1;
    bool secondary = false;

    std::vector<Real> local_stos;
    std::vector<VariableName> local_species_list;

    // Find every single term in this reaction (species and operators)
    while (re_terms.FindAndConsume(&single_reaction, &term))
    {
      // Separating the stoichiometric coefficients from species
      if (re_coeff_and_species.PartialMatch(term, &coeff_str, &species))
      {
        if (coeff_str.length())
          coeff = std::stod(coeff_str);
        else
          coeff = 1.0;

        coeff *= sign;

        if (secondary)
          _eq_species.push_back(species);
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

    _stos.push_back(local_stos);
    _primary_species_involved.push_back(local_species_list);
  }

  // Start picking out primary species and coupled primary species and assigning
  // corresponding stoichiomentric coefficients
  for (unsigned int i = 0; i < _primary_species.size(); ++i)
  {
    _sto_u[i].resize(_num_reactions, 0.0);
    _sto_v[i].resize(_num_reactions);
    _coupled_v[i].resize(_num_reactions);
    _weights[i].resize(_num_reactions, 0.0);

    _primary_participation[i].resize(_num_reactions, false);
    for (unsigned int j = 0; j < _num_reactions; ++j)
    {
      for (unsigned int k = 0; k < _primary_species_involved[j].size(); ++k)
        if (_primary_species_involved[j][k] == _primary_species[i])
          _primary_participation[i][j] = true;

      if (_primary_participation[i][j])
      {
        for (unsigned int k = 0; k < _primary_species_involved[j].size(); ++k)
        {
          if (_primary_species_involved[j][k] == _primary_species[i])
          {
            _sto_u[i][j] = _stos[j][k];
            _weights[i][j] = _stos[j][k];
          }
          else
          {
            _sto_v[i][j].push_back(_stos[j][k]);
            _coupled_v[i][j].push_back(_primary_species_involved[j][k]);
          }
        }
      }
    }
  }

  // Print out details of the equilibrium reactions to the console
  _console << "Aqueous equilibrium reactions:\n";
  for (unsigned int i = 0; i < _num_reactions; ++i)
    _console << "  Reaction " << i + 1 << ": " << _reactions[i] << ", log10(Keq) = " << _eq_const[i]
             << "\n";
  _console << std::endl;
}

void
AddCoupledEqSpeciesAction::act()
{
  if (_current_task == "add_kernel")
  {
    // Add Kernels for each primary species
    for (unsigned int i = 0; i < _primary_species.size(); ++i)
    {
      for (unsigned int j = 0; j < _num_reactions; ++j)
      {
        if (_primary_participation[i][j])
        {
          InputParameters params_sub = _factory.getValidParams("CoupledBEEquilibriumSub");
          params_sub.set<NonlinearVariableName>("variable") = _primary_species[i];
          params_sub.set<Real>("weight") = _weights[i][j];
          params_sub.defaultCoupledValue("log_k", _eq_const[j]);
          params_sub.set<Real>("sto_u") = _sto_u[i][j];
          params_sub.set<std::vector<Real>>("sto_v") = _sto_v[i][j];
          params_sub.set<std::vector<VariableName>>("v") = _coupled_v[i][j];
          _problem->addKernel("CoupledBEEquilibriumSub",
                              _primary_species[i] + "_" + _eq_species[j] + "_sub",
                              params_sub);

          InputParameters params_cd = _factory.getValidParams("CoupledDiffusionReactionSub");
          params_cd.set<NonlinearVariableName>("variable") = _primary_species[i];
          params_cd.set<Real>("weight") = _weights[i][j];
          params_cd.defaultCoupledValue("log_k", _eq_const[j]);
          params_cd.set<Real>("sto_u") = _sto_u[i][j];
          params_cd.set<std::vector<Real>>("sto_v") = _sto_v[i][j];
          params_cd.set<std::vector<VariableName>>("v") = _coupled_v[i][j];
          _problem->addKernel("CoupledDiffusionReactionSub",
                              _primary_species[i] + "_" + _eq_species[j] + "_cd",
                              params_cd);

          // If pressure is coupled, add a CoupledConvectionReactionSub Kernel as well
          if (_pars.isParamValid("pressure"))
          {
            InputParameters params_conv = _factory.getValidParams("CoupledConvectionReactionSub");
            params_conv.set<NonlinearVariableName>("variable") = _primary_species[i];
            params_conv.set<Real>("weight") = _weights[i][j];
            params_conv.defaultCoupledValue("log_k", _eq_const[j]);
            params_conv.set<Real>("sto_u") = _sto_u[i][j];
            params_conv.set<std::vector<Real>>("sto_v") = _sto_v[i][j];
            params_conv.set<std::vector<VariableName>>("v") = _coupled_v[i][j];
            params_conv.set<std::vector<VariableName>>("p") = _pressure_var;
            params_conv.set<RealVectorValue>("gravity") = _gravity;
            _problem->addKernel("CoupledConvectionReactionSub",
                                _primary_species[i] + "_" + _eq_species[j] + "_conv",
                                params_conv);
          }
        }
      }
    }
  }

  if (_current_task == "add_aux_kernel")
  {
    // Add AqueousEquilibriumRxnAux AuxKernels for equilibrium species
    for (unsigned int j = 0; j < _num_reactions; ++j)
    {
      if (_aux_species.find(_eq_species[j]) != _aux_species.end())
      {
        InputParameters params_eq = _factory.getValidParams("AqueousEquilibriumRxnAux");
        params_eq.set<AuxVariableName>("variable") = _eq_species[j];
        params_eq.defaultCoupledValue("log_k", _eq_const[j]);
        params_eq.set<std::vector<Real>>("sto_v") = _stos[j];
        params_eq.set<std::vector<VariableName>>("v") = _primary_species_involved[j];
        _problem->addAuxKernel("AqueousEquilibriumRxnAux", "aux_" + _eq_species[j], params_eq);
      }
    }
  }
}
