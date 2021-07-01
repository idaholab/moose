//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddCoupledSolidKinSpeciesAction.h"
#include "MooseUtils.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"
#include "Parser.h"
#include <algorithm>

// Regular expression includes
#include "pcrecpp.h"

registerMooseAction("ChemicalReactionsApp", AddCoupledSolidKinSpeciesAction, "add_kernel");

registerMooseAction("ChemicalReactionsApp", AddCoupledSolidKinSpeciesAction, "add_aux_kernel");

InputParameters
AddCoupledSolidKinSpeciesAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<std::vector<NonlinearVariableName>>("primary_species",
                                                              "The list of primary species to add");
  params.addParam<std::vector<AuxVariableName>>(
      "secondary_species", "The list of solid kinetic species to be output as aux variables");
  params.addRequiredParam<std::string>("kin_reactions", "The list of solid kinetic reactions");
  params.addRequiredParam<std::vector<Real>>("log10_keq",
                                             "The list of equilibrium constants for all reactions");
  params.addRequiredParam<std::vector<Real>>(
      "specific_reactive_surface_area",
      "The list of specific reactive surface area for all minerals (m^2/L)");
  params.addRequiredParam<std::vector<Real>>(
      "kinetic_rate_constant", "The list of kinetic rate constant for all reactions (mol/m^2/s)");
  params.addRequiredParam<std::vector<Real>>(
      "activation_energy", "The list of activation energy values for all reactions (J/mol)");
  params.addParam<Real>("gas_constant", 8.314, "Gas constant. Default is 8.314 (J/mol/K)");
  params.addRequiredParam<std::vector<Real>>(
      "reference_temperature", "The list of reference temperatures for all reactions (K)");
  params.addRequiredCoupledVar("system_temperature",
                               "The system temperature for all reactions (K)");
  params.addClassDescription("Adds solid kinetic Kernels and AuxKernels for primary species");
  return params;
}

AddCoupledSolidKinSpeciesAction::AddCoupledSolidKinSpeciesAction(const InputParameters & params)
  : Action(params),
    _primary_species(getParam<std::vector<NonlinearVariableName>>("primary_species")),
    _secondary_species(getParam<std::vector<AuxVariableName>>("secondary_species")),
    _kinetic_species_involved(_primary_species.size()),
    _weights(_primary_species.size()),
    _input_reactions(getParam<std::string>("kin_reactions")),
    _logk(getParam<std::vector<Real>>("log10_keq")),
    _r_area(getParam<std::vector<Real>>("specific_reactive_surface_area")),
    _ref_kconst(getParam<std::vector<Real>>("kinetic_rate_constant")),
    _e_act(getParam<std::vector<Real>>("activation_energy")),
    _gas_const(getParam<Real>("gas_constant")),
    _ref_temp(getParam<std::vector<Real>>("reference_temperature")),
    _sys_temp(getParam<std::vector<VariableName>>("system_temperature"))
{
  // Note: as the reaction syntax has changed, check to see if the old syntax has
  // been used and throw an informative error. The number of = signs should be one
  // more than the number of commas, while the smallest number of spaces possible is 2
  bool old_syntax = false;
  if (std::count(_input_reactions.begin(), _input_reactions.end(), '=') !=
      std::count(_input_reactions.begin(), _input_reactions.end(), ',') + 1)
    old_syntax = true;

  if (std::count(_input_reactions.begin(), _input_reactions.end(), ' ') < 2)
    old_syntax = true;

  if (old_syntax)
    mooseError("Old solid kinetic reaction syntax present.\nReactions should now be comma "
               "separated, and must have spaces between species and +/-/= operators.\n"
               "See #9972 for details");

  // Parse the kinetic reactions
  pcrecpp::RE re_reactions("(.+?)" // A single reaction (any character until the comma delimiter)
                           "(?:,\\s*|$)" // comma or end of string
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

  // Parse reaction network to extract each individual reaction
  while (re_reactions.FindAndConsume(&input, &single_reaction_str))
    _reactions.push_back(single_reaction_str);

  _num_reactions = _reactions.size();

  if (_num_reactions == 0)
    mooseError("No solid kinetic reaction provided!");

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
          _solid_kinetic_species.push_back(species);
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
    for (unsigned int j = 0; j < _num_reactions; ++j)
    {
      for (unsigned int k = 0; k < _primary_species_involved[j].size(); ++k)
        if (_primary_species_involved[j][k] == _primary_species[i])
        {
          _weights[i].push_back(_stos[j][k]);
          _kinetic_species_involved[i].push_back(_solid_kinetic_species[j]);
        }
    }

  // Print out details of the solid kinetic reactions to the console
  _console << "Solid kinetic reactions:\n";
  for (unsigned int i = 0; i < _num_reactions; ++i)
    _console << "  Reaction " << i + 1 << ": " << _reactions[i] << "\n";
  _console << std::endl;

  // Check that all secondary species read from the reaction network have been added
  // as AuxVariables. Note: can't sort the _solid_kinetic_species vector as it throws
  // out the species and coefficient vectors so use std::is_permutation
  if (!std::is_permutation(
          _secondary_species.begin(), _secondary_species.end(), _solid_kinetic_species.begin()))
    mooseError("All solid kinetic species must be added as secondary species");

  // Check that the size of property vectors is equal to the number of reactions
  if (_logk.size() != _num_reactions)
    mooseError("The number of values entered for log10_keq is not equal to the number of solid "
               "kinetic reactions");
  if (_r_area.size() != _num_reactions)
    mooseError("The number of values entered for specific_reactive_surface_area is not equal to "
               "the number of solid kinetic reactions");
  if (_ref_kconst.size() != _num_reactions)
    mooseError("The number of values entered for kinetic_rate_constant is not equal to the number "
               "of solid kinetic reactions");
  if (_e_act.size() != _num_reactions)
    mooseError("The number of values entered for activation_energy is not equal to the number of "
               "solid kinetic reactions");
  if (_ref_temp.size() != _num_reactions)
    mooseError("The number of values entered for reference_temperature is not equal to the number "
               "of solid kinetic reactions");
}

void
AddCoupledSolidKinSpeciesAction::act()
{
  if (_current_task == "add_kernel")
  {
    // Add Kernels for each primary species
    for (unsigned int i = 0; i < _primary_species.size(); ++i)
    {
      InputParameters params_kin = _factory.getValidParams("CoupledBEKinetic");
      params_kin.set<NonlinearVariableName>("variable") = _primary_species[i];
      params_kin.set<std::vector<Real>>("weight") = _weights[i];
      params_kin.set<std::vector<VariableName>>("v") = _kinetic_species_involved[i];
      _problem->addKernel("CoupledBEKinetic", _primary_species[i] + "_" + "_kin", params_kin);
    }
  }

  if (_current_task == "add_aux_kernel")
  {
    // Add AuxKernels for each solid kinetic species
    for (unsigned int i = 0; i < _num_reactions; ++i)
    {
      InputParameters params_kin = _factory.getValidParams("KineticDisPreConcAux");
      params_kin.set<AuxVariableName>("variable") = _solid_kinetic_species[i];
      params_kin.defaultCoupledValue("log_k", _logk[i]);
      params_kin.set<Real>("r_area") = _r_area[i];
      params_kin.set<Real>("ref_kconst") = _ref_kconst[i];
      params_kin.set<Real>("e_act") = _e_act[i];
      params_kin.set<Real>("gas_const") = _gas_const;
      params_kin.set<Real>("ref_temp") = _ref_temp[i];
      params_kin.set<std::vector<VariableName>>("sys_temp") = _sys_temp;
      params_kin.set<std::vector<Real>>("sto_v") = _stos[i];
      params_kin.set<std::vector<VariableName>>("v") = _primary_species_involved[i];
      _problem->addAuxKernel(
          "KineticDisPreConcAux", "aux_" + _solid_kinetic_species[i], params_kin);
    }
  }
}
