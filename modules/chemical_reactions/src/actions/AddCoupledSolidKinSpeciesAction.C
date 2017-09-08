/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddCoupledSolidKinSpeciesAction.h"
#include "MooseUtils.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"

#include <sstream>

template <>
InputParameters
validParams<AddCoupledSolidKinSpeciesAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName>>("primary_species",
                                                              "The list of primary species to add");
  params.addRequiredParam<std::vector<std::string>>("kin_reactions",
                                                    "The list of solid kinetic reactions");
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
  params.addClassDescription("Adds Kernels for primary kinetic species");
  return params;
}

AddCoupledSolidKinSpeciesAction::AddCoupledSolidKinSpeciesAction(const InputParameters & params)
  : Action(params),
    _primary_species(getParam<std::vector<NonlinearVariableName>>("primary_species")),
    _reactions(getParam<std::vector<std::string>>("kin_reactions")),
    _logk(getParam<std::vector<Real>>("log10_keq")),
    _r_area(getParam<std::vector<Real>>("specific_reactive_surface_area")),
    _ref_kconst(getParam<std::vector<Real>>("kinetic_rate_constant")),
    _e_act(getParam<std::vector<Real>>("activation_energy")),
    _gas_const(getParam<Real>("gas_constant")),
    _ref_temp(getParam<std::vector<Real>>("reference_temperature")),
    _sys_temp(getParam<std::vector<VariableName>>("system_temperature"))
{
  // Check that the size of property vectors is equal to the number of reactions
  if (_logk.size() != _reactions.size())
    mooseError("The number of values entered for log10_keq is not equal to the number of solid "
               "kinetic reactions");
  if (_r_area.size() != _reactions.size())
    mooseError("The number of values entered for specific_reactive_surface_area is not equal to "
               "the number of solid kinetic reactions");
  if (_ref_kconst.size() != _reactions.size())
    mooseError("The number of values entered for kinetic_rate_constant is not equal to the number "
               "of solid kinetic reactions");
  if (_e_act.size() != _reactions.size())
    mooseError("The number of values entered for activation_energy is not equal to the number "
               "of solid kinetic reactions");
  if (_ref_temp.size() != _reactions.size())
    mooseError("The number of values entered for reference_temperature is not equal to the number "
               "of solid kinetic reactions");
}

void
AddCoupledSolidKinSpeciesAction::act()
{
  mooseDoOnce(printReactions());

  std::vector<bool> primary_participation(_primary_species.size(), false);
  std::vector<std::string> solid_kin_species(_reactions.size());
  std::vector<Real> weight;

  // Loop through reactions
  for (unsigned int j = 0; j < _reactions.size(); ++j)
  {
    std::vector<std::string> tokens;

    // Parsing each reaction
    MooseUtils::tokenize(_reactions[j], tokens, 1, "+=");
    if (tokens.size() == 0)
      mooseError("Empty reaction specified.");

    std::vector<Real> stos(tokens.size() - 1);
    std::vector<VariableName> rxn_species(tokens.size() - 1);

    for (unsigned int k = 0; k < tokens.size(); ++k)
    {
      std::vector<std::string> stos_primary_species;
      MooseUtils::tokenize(tokens[k], stos_primary_species, 1, "()");
      if (stos_primary_species.size() == 2)
      {
        Real coef;
        std::istringstream iss(stos_primary_species[0]);
        iss >> coef;
        stos[k] = coef;
        rxn_species[k] = stos_primary_species[1];

        // Check the participation of primary species
        for (unsigned int i = 0; i < _primary_species.size(); ++i)
          if (rxn_species[k] == _primary_species[i])
            primary_participation[i] = true;
      }
      else
        solid_kin_species[j] = stos_primary_species[0];
    }

    if (_current_task == "add_kernel")
    {
      for (unsigned int i = 0; i < _primary_species.size(); ++i)
      {
        if (primary_participation[i])
        {
          // Assigning the stoichiometrics based on parsing
          for (unsigned int m = 0; m < rxn_species.size(); ++m)
            if (rxn_species[m] == _primary_species[i])
              weight.push_back(stos[m]);

          std::vector<VariableName> coupled_var = {solid_kin_species[j]};

          // Building kernels for solid kinetic species
          InputParameters params_kin = _factory.getValidParams("CoupledBEKinetic");
          params_kin.set<NonlinearVariableName>("variable") = _primary_species[i];
          params_kin.set<std::vector<Real>>("weight") = weight;
          params_kin.set<std::vector<VariableName>>("v") = coupled_var;
          _problem->addKernel("CoupledBEKinetic",
                              _primary_species[i] + "_" + solid_kin_species[j] + "_kin",
                              params_kin);
        }
      }
    }

    if (_current_task == "add_aux_kernel")
    {
      InputParameters params_kin = _factory.getValidParams("KineticDisPreConcAux");
      params_kin.set<AuxVariableName>("variable") = solid_kin_species[j];
      params_kin.set<Real>("log_k") = _logk[j];
      params_kin.set<Real>("r_area") = _r_area[j];
      params_kin.set<Real>("ref_kconst") = _ref_kconst[j];
      params_kin.set<Real>("e_act") = _e_act[j];
      params_kin.set<Real>("gas_const") = _gas_const;
      params_kin.set<Real>("ref_temp") = _ref_temp[j];
      params_kin.set<std::vector<VariableName>>("sys_temp") = _sys_temp;
      params_kin.set<std::vector<Real>>("sto_v") = stos;
      params_kin.set<std::vector<VariableName>>("v") = rxn_species;
      _problem->addAuxKernel("KineticDisPreConcAux", "aux_" + solid_kin_species[j], params_kin);
    }
  }
}

void
AddCoupledSolidKinSpeciesAction::printReactions() const
{
  _console << "Solid kinetic reactions:\n";

  for (unsigned int i = 0; i < _reactions.size(); ++i)
    _console << _reactions[i] << "\n";

  _console << "\n";
}
