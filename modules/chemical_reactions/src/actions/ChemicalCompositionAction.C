//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChemicalCompositionAction.h"
#include "ThermochimicaUtils.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "AddVariableAction.h"
#include "libmesh/string_to_enum.h"
#include "BlockRestrictable.h"
#include "InputParameterWarehouse.h"

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#include "checkUnits.h"
#endif

registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_variable");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_aux_variable");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_ic");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_user_object");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_aux_kernel");

InputParameters
ChemicalCompositionAction::validParams()
{
  InputParameters params = Action::validParams();
  params += BlockRestrictable::validParams();

  ThermochimicaUtils::addClassDescription(params,
                                          "Sets up the thermodynamic model and variables for the "
                                          "thermochemistry solve using Thermochimica.");

  // Required variables
  params.addParam<std::vector<std::string>>(
      "elements", {"ALL"}, "List of chemical elements (or ALL)");
  params.addCoupledVar("temperature", "Name of temperature variable");
  params.addCoupledVar("pressure", "Name of pressure variable");
  MooseEnum reinit_type("none time nodal", "nodal");
  params.addParam<MooseEnum>(
      "reinitialization_type", reinit_type, "Reinitialization scheme to use with Thermochimica");
  params.addParam<FileName>("initial_values", "The CSV file name with initial conditions.");
  params.addParam<FileName>("thermofile", "Thermodynamics model file");

  MooseEnum tUnit("K C F R");
  params.addParam<MooseEnum>("tunit", tUnit, "Temperature Unit");
  MooseEnum pUnit("atm psi bar Pa kPa");
  params.addParam<MooseEnum>("punit", pUnit, "Pressure Unit");
  MooseEnum mUnit(
      "mole_fraction atom_fraction atoms moles gram-atoms mass_fraction kilograms grams pounds");
  params.addParam<MooseEnum>("munit", mUnit, "Mass Unit");
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.addParam<ExecFlagEnum>(
      "execute_on", exec_enum, "When to execute the ThermochimicaData UO");
  params.addParam<bool>("is_fv", false, "Should the variables set up by action be of FV type");

  params.addParam<std::vector<std::string>>("output_phases", {}, "List of phases to be output");
  params.addParam<std::vector<std::string>>(
      "output_species", {}, "List species for which concentration in the phases is needed");
  MooseEnum mUnit_op("moles mole_fraction");
  params.addParam<MooseEnum>(
      "output_species_unit", mUnit_op, "Mass unit for output species: mole_fractions or moles");
  params.addParam<std::vector<std::string>>(
      "output_element_potentials",
      {},
      "List of chemical elements for which chemical potentials are requested");
  params.addParam<std::vector<std::string>>(
      "output_vapor_pressures",
      {},
      "List of gas phase species for which vapor pressures are requested");
  params.addParam<std::vector<std::string>>(
      "output_element_phases",
      {},
      "List of elements whose molar amounts in specific phases are requested");
  params.addParam<std::string>(
      "uo_name", "Thermochimica", "Name of the ThermochimicaDataUserObject.");
  return params;
}

ChemicalCompositionAction::ChemicalCompositionAction(const InputParameters & parameters)
  : Action(parameters)
{
  const auto & params = _app.getInputParameterWarehouse().getInputParameters();
  InputParameters & pars(*(params.find(uniqueActionName())->second.get()));

  // check if a container block with common parameters is found
  auto action = _awh.getActions<CommonChemicalCompositionAction>();
  if (action.size() == 1)
    pars.applyParameters(action[0]->parameters());

  if (!isParamValid("tunit"))
    paramError(
        "tunit",
        "The temperature unit must be specified for Thermochimica objects to be constructed");

  if (!isParamValid("punit"))
    paramError("punit",
               "The pressure unit must be specified for Thermochimica objects to be constructed");

  if (!isParamValid("munit"))
    paramError("munit",
               "The mass unit must be specified for Thermochimica objects to be constructed");

  if (!isParamValid("temperature"))
    paramError("temperature",
               "Temperature variable must be specified for this object to be constructed");

  if ((isParamValid("output_species") || isParamValid("output_element_phases")) &&
      !isParamValid("output_species_unit"))
    paramError(
        "output_species_unit",
        "Output mass unit must be specified for Thermochimica user object to be constructed");

  ThermochimicaUtils::checkLibraryAvailability(*this);

#ifdef THERMOCHIMICA_ENABLED
  // Initialize database in Thermochimica
  if (isParamValid("thermofile"))
  {
    const auto thermo_file = getParam<FileName>("thermofile");

    if (thermo_file.length() > 1024)
      paramError("thermofile",
                 "Path exceeds Thermochimica's maximal permissible length of 1024 with ",
                 thermo_file.length(),
                 " characters: ",
                 thermo_file);

    Thermochimica::setThermoFilename(thermo_file);

    // Read in thermodynamics model for setting up variables
    Thermochimica::parseThermoFile();

    const auto idbg = Thermochimica::checkInfoThermo();
    if (idbg != 0)
      paramError("thermofile", "Thermochimica data file cannot be parsed. ", idbg);
  }

  // Set thermochimica units
  auto tunit = Moose::stringify(getParam<MooseEnum>("tunit"));
  Thermochimica::checkTemperature(tunit);
  Thermochimica::setUnitTemperature(tunit);
  int idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
    paramError("tunit", "Cannot set temperature unit in Thermochimica", idbg);

  auto punit = Moose::stringify(getParam<MooseEnum>("punit"));
  Thermochimica::checkPressure(punit);
  Thermochimica::setUnitPressure(punit);
  idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
    paramError("punit", "Cannot set pressure unit in Thermochimica", idbg);

  auto munit = Moose::stringify(getParam<MooseEnum>("munit"));
  std::replace(munit.begin(), munit.end(), '_', ' ');
  Thermochimica::checkMass(munit);
  Thermochimica::setUnitMass(munit);
  idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
    paramError("munit", "Cannot set mass unit in Thermochimica", idbg);

  _elements = getParam<std::vector<std::string>>("elements");
  if (_elements.size() == 1 && _elements[0] == "ALL")
  {
    _elements.resize(Thermochimica::getNumberElementsDatabase());
    _elements = Thermochimica::getElementsDatabase();
    mooseInfo("Thermochimica elements: 'ALL' specified in input file. Using: ",
              Moose::stringify(_elements));
  }
  else
  {
    std::vector<std::string> db_elements(Thermochimica::getNumberElementsDatabase());
    db_elements = Thermochimica::getElementsDatabase();
    for (const auto i : index_range(_elements))
      if (std::find(db_elements.begin(), db_elements.end(), _elements[i]) == db_elements.end())
        paramError("elements", "Element '", _elements[i], "' was not found in the database.");
  }
  _element_ids.resize(_elements.size());
  for (const auto i : index_range(_elements))
    _element_ids[i] = Thermochimica::atomicNumber(_elements[i]);

  // I want to check all the input parameters here and have a list of possible phases and species
  // for setting up the Aux variables with "ALL" option

  // Temporarily set Thermochimica state space to get the list of possible phases and species
  Thermochimica::setTemperaturePressure(1000.0, 1.0);
  Thermochimica::setElementMass(0, 0.0);

  for (const auto i : make_range(_elements.size()))
    Thermochimica::setElementMass(Thermochimica::atomicNumber(_elements[i]), 1.0);

  Thermochimica::setup();

  if (isParamValid("output_phases"))
  {
    _phases = getParam<std::vector<std::string>>("output_phases");
    if (_phases.size() == 1 && _phases[0] == "ALL")
    {
      auto [soln_phases, stoich_phases] = Thermochimica::getNumberPhasesSystem();
      _phases.resize(soln_phases + stoich_phases);
      _phases = Thermochimica::getPhaseNamesSystem();
      mooseInfo("ChemicalCompositionAction phases: 'ALL' specified in input file. Using: ",
                Moose::stringify(_phases));
    }
    else
    {
      auto db_phases = Thermochimica::getPhaseNamesSystem();
      for (const auto i : index_range(_phases))
        if (std::find(db_phases.begin(), db_phases.end(), _phases[i]) == db_phases.end())
          paramError("output_phases", "Phase '", _phases[i], "' was not found in the simulation.");
    }
  }

  if (isParamValid("output_species"))
  {
    auto species = getParam<std::vector<std::string>>("output_species");
    auto db_phases = Thermochimica::getPhaseNamesSystem();
    auto n_db_species = Thermochimica::getNumberSpeciesSystem();
    auto db_species = Thermochimica::getSpeciesSystem();
    for (auto i : index_range(n_db_species))
      if (Thermochimica::isPhaseMQM(i))
      {
        auto [pairs, quads, idbg] =
            Thermochimica::getMqmqaNumberPairsQuads(Thermochimica::getPhaseNamesSystem()[i]);
        n_db_species[i] = pairs;
      }

    if (species.size() == 1 && species[0] == "ALL")
    {
      if (!n_db_species.empty())
        species.resize(n_db_species.back());
      else
        mooseInfo("ChemicalCompositionAction species: 'ALL' specified in input file. Thermochimica "
                  "returned no possible species.");

      species.clear();
      _tokenized_species.clear();
      for (const auto i : make_range(db_species.size()))
        for (const auto j : index_range(db_species[i]))
        {
          species.push_back(db_phases[i] + ":" + db_species[i][j]);
          _tokenized_species.push_back(std::make_pair(db_phases[i], db_species[i][j]));
        }
      mooseInfo("ChemicalCompositionAction species: 'ALL' specified in input file. Using: ",
                Moose::stringify(species));
    }
    else
      for (const auto i : index_range(species))
      {
        _tokenized_species.resize(species.size());
        std::vector<std::string> tokens;
        MooseUtils::tokenize(species[i], tokens, 1, ":");
        if (tokens.size() == 1)
          paramError("output_species", "No ':' separator found in variable '", species[i], "'");

        auto phase_index = std::find(db_phases.begin(), db_phases.end(), tokens[0]);
        if (phase_index == db_phases.end())
          paramError("output_species",
                     "Phase '",
                     tokens[0],
                     "' of output species '",
                     species[i],
                     "' not found in the simulation.");
        auto sp = db_species[std::distance(db_phases.begin(), phase_index)];
        if (std::find(sp.begin(), sp.end(), tokens[1]) == sp.end())
          paramError(
              "output_species", "Species '", tokens[1], "' was not found in the simulation.");
        _tokenized_species[i] = std::make_pair(tokens[0], tokens[1]);
      }
  }

  if (isParamValid("output_element_potentials"))
  {
    auto element_potentials = getParam<std::vector<std::string>>("output_element_potentials");
    if (element_potentials.size() == 1 && element_potentials[0] == "ALL")
    {
      _tokenized_element_potentials.resize(_elements.size());
      _tokenized_element_potentials = _elements;
      element_potentials.resize(_elements.size());
      for (const auto i : index_range(_elements))
        element_potentials[i] = "mu:" + _elements[i];
      mooseInfo(
          "ChemicalCompositionAction element potentials: 'ALL' specified in input file. Using: ",
          Moose::stringify(element_potentials));
    }
    else
    {
      _tokenized_element_potentials.resize(element_potentials.size());
      for (const auto i : index_range(element_potentials))
      {
        std::vector<std::string> tokens;
        MooseUtils::tokenize(element_potentials[i], tokens, 1, ":");
        if (tokens.size() == 1)
          paramError("output_element_potentials",
                     "No ':' separator found in variable '",
                     element_potentials[i],
                     "'");
        if (std::find(_elements.begin(), _elements.end(), tokens[1]) == _elements.end())
          paramError("output_element_potentials",
                     "Element '",
                     tokens[1],
                     "' was not found in the simulation.");
        _tokenized_element_potentials[i] = tokens[1];
      }
    }
  }

  if (isParamValid("output_vapor_pressures"))
  {
    auto vapor_pressures = getParam<std::vector<std::string>>("output_vapor_pressures");
    if (!Thermochimica::isPhaseGas(0))
      paramError("output_vapor_pressures",
                 "No gas phase found in the simulation. Cannot output vapor pressures.");
    if (vapor_pressures.size() == 1 && vapor_pressures[0] == "ALL")
    {
      vapor_pressures.resize(Thermochimica::getNumberSpeciesSystem()[0]);
      _tokenized_vapor_species.resize(Thermochimica::getNumberSpeciesSystem()[0]);
      auto db_gas_species = Thermochimica::getSpeciesInPhase(0);
      auto gas_name = Thermochimica::getPhaseNamesSystem()[0];
      for (const auto i : index_range(db_gas_species))
      {
        vapor_pressures[i] = "vp:" + gas_name + ':' + db_gas_species[i];
        _tokenized_vapor_species[i] = std::make_pair(gas_name, db_gas_species[i]);
      }
      mooseInfo("ChemicalCompositionAction vapor pressures: 'ALL' specified in input file. Using: ",
                Moose::stringify(vapor_pressures));
    }
    else
    {
      auto db_gas_species = Thermochimica::getSpeciesInPhase(0);
      _tokenized_vapor_species.resize(vapor_pressures.size());
      for (const auto i : index_range(vapor_pressures))
      {
        std::vector<std::string> tokens;
        MooseUtils::tokenize(vapor_pressures[i], tokens, 1, ":");
        if (tokens.size() == 1)
          paramError("output_vapor_pressures",
                     "No ':' separator found in variable '",
                     vapor_pressures[i],
                     "'");
        if (tokens[1] != Thermochimica::getPhaseNamesSystem()[0])
          paramError("output_vapor_pressures",
                     "Phase '",
                     tokens[1],
                     "' of vapor species '",
                     vapor_pressures[i],
                     "' is not a gas phase. Cannot calculate vapor pressure.");
        if (std::find(db_gas_species.begin(), db_gas_species.end(), tokens[2]) ==
            db_gas_species.end())
          paramError("output_vapor_pressures",
                     "Species '",
                     tokens[2],
                     "' was not found in the gas phase of simulation.");
        _tokenized_vapor_species[i] = std::make_pair(tokens[1], tokens[2]);
      }
    }
  }

  if (isParamValid("output_element_phases"))
  {
    auto element_phases = getParam<std::vector<std::string>>("output_element_phases");
    auto db_phases = Thermochimica::getPhaseNamesSystem();
    if (element_phases.size() == 1 && element_phases[0] == "ALL")
    {
      element_phases.resize(_elements.size() * db_phases.size());
      _tokenized_phase_elements.resize(_elements.size() * db_phases.size());
      for (const auto i : index_range(db_phases))
        for (const auto j : index_range(_elements))
        {
          element_phases[i * _elements.size() + j] = db_phases[i] + ':' + _elements[j];
          _tokenized_phase_elements[i * _elements.size() + j] =
              std::make_pair(db_phases[i], _elements[j]);
        }
      mooseInfo(
          "ChemicalCompositionAction elements in phase: 'ALL' specified in input file. Using: ",
          Moose::stringify(element_phases));
    }
    else
    {
      _tokenized_phase_elements.resize(element_phases.size());
      for (const auto i : index_range(element_phases))
      {
        std::vector<std::string> tokens;
        MooseUtils::tokenize(element_phases[i], tokens, 1, ":");
        if (tokens.size() == 1)
          paramError("output_element_phases",
                     "No ':' separator found in variable '",
                     element_phases[i],
                     "'");
        if (std::find(db_phases.begin(), db_phases.end(), tokens[1]) == db_phases.end())
          paramError("output_element_phases",
                     "Phase '",
                     tokens[1],
                     "' of '",
                     element_phases[i],
                     "' not found in the simulation.");
        if (std::find(_elements.begin(), _elements.end(), tokens[2]) == _elements.end())
          paramError("output_element_phases",
                     "Element '",
                     tokens[2],
                     "' was not found in the simulation.");
        _tokenized_phase_elements[i] = std::make_pair(tokens[1], tokens[2]);
      }
    }
  }

  Thermochimica::resetThermoAll();

#endif
}

void
ChemicalCompositionAction::act()
{
#ifdef THERMOCHIMICA_ENABLED
  //
  // Add AuxVariables
  //
  if (_current_task == "add_aux_variable")
  {
    auto aux_var_type = AddVariableAction::variableType(
        FEType(Utility::string_to_enum<Order>(_problem->mesh().hasSecondOrderElements() ? "SECOND"
                                                                                        : "FIRST"),
               Utility::string_to_enum<libMesh::FEFamily>("LAGRANGE")),
        /* is_fv = */ getParam<bool>("is_fv"),
        /* is_array = */ false);
    auto params = _factory.getValidParams(aux_var_type);

    for (const auto i : index_range(_elements))
      _problem->addAuxVariable(aux_var_type, _elements[i], params);

    for (const auto i : index_range(_phases))
      _problem->addAuxVariable(aux_var_type, _phases[i], params);

    for (const auto i : index_range(_tokenized_species))
      _problem->addAuxVariable(
          aux_var_type, Moose::stringify(_tokenized_species[i], /* delim = */ ":"), params);

    for (const auto i : index_range(_tokenized_element_potentials))
      _problem->addAuxVariable(aux_var_type, "mu:" + _tokenized_element_potentials[i], params);

    for (const auto i : index_range(_tokenized_vapor_species))
      _problem->addAuxVariable(aux_var_type,
                               "vp:" +
                                   Moose::stringify(_tokenized_vapor_species[i], /* delim = */ ":"),
                               params);

    for (const auto i : index_range(_tokenized_phase_elements))
      _problem->addAuxVariable(
          aux_var_type,
          "ep:" + Moose::stringify(_tokenized_phase_elements[i], /* delim = */ ":"),
          params);
  }

  //
  // Set up initial conditions from a file
  //
  if (_current_task == "add_ic" && isParamValid("initial_values"))
  {
    readCSV();
    for (auto it : _initial_conditions)
    {
      const std::string class_name = "ConstantIC";
      auto params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = it.first;
      params.set<Real>("value") = it.second;
      _problem->addInitialCondition(class_name, it.first + "_ic", params);
    }
  }

  //
  // Set up user object
  //
  if (_current_task == "add_user_object")
  {
    std::string uo_name = getParam<std::string>("uo_name");

    if (isParamValid("block") && !isParamSetByUser("uo_name"))
      uo_name += "_" + Moose::stringify(getParam<std::vector<SubdomainName>>("block"));

    const auto uo_type =
        getParam<bool>("is_fv") ? "ThermochimicaElementData" : "ThermochimicaNodalData";

    auto uo_params = _factory.getValidParams(uo_type);

    std::copy(_elements.begin(),
              _elements.end(),
              std::back_inserter(uo_params.set<std::vector<VariableName>>("elements")));

    if (isParamValid("output_phases"))
      std::copy(_phases.begin(),
                _phases.end(),
                std::back_inserter(uo_params.set<std::vector<VariableName>>("output_phases")));

    if (isParamValid("output_species"))
    {
      std::vector<std::string> species;
      for (auto token : _tokenized_species)
        species.push_back(Moose::stringify(token, ":"));
      uo_params.set<std::vector<VariableName>>("output_species")
          .insert(uo_params.set<std::vector<VariableName>>("output_species").end(),
                  species.begin(),
                  species.end());
    }

    if (isParamValid("output_element_potentials"))
    {
      std::vector<std::string> element_potentials;
      for (auto token : _tokenized_element_potentials)
        element_potentials.push_back("mu:" + token);
      uo_params.set<std::vector<VariableName>>("output_element_potentials")
          .insert(uo_params.set<std::vector<VariableName>>("output_element_potentials").end(),
                  element_potentials.begin(),
                  element_potentials.end());
    }

    if (isParamValid("output_vapor_pressures"))
    {
      std::vector<std::string> vapor_pressures;
      for (auto token : _tokenized_vapor_species)
        vapor_pressures.push_back("vp:" + Moose::stringify(token, ":"));
      uo_params.set<std::vector<VariableName>>("output_vapor_pressures")
          .insert(uo_params.set<std::vector<VariableName>>("output_vapor_pressures").end(),
                  vapor_pressures.begin(),
                  vapor_pressures.end());
    }

    if (isParamValid("output_element_phases"))
    {
      std::vector<std::string> element_phases;
      for (auto token : _tokenized_phase_elements)
        element_phases.push_back("ep:" + Moose::stringify(token, ":"));
      uo_params.set<std::vector<VariableName>>("output_element_phases")
          .insert(uo_params.set<std::vector<VariableName>>("output_element_phases").end(),
                  element_phases.begin(),
                  element_phases.end());
    }
    uo_params.set<std::vector<VariableName>>("temperature") =
        getParam<std::vector<VariableName>>("temperature");

    uo_params.set<ChemicalCompositionAction *>("_chemical_composition_action") = this;

    uo_params.set<FileName>("thermofile") = getParam<FileName>("thermofile");

    uo_params.set<MooseEnum>("reinit_type") = getParam<MooseEnum>("reinitialization_type");

    uo_params.set<MooseEnum>("output_species_unit") = getParam<MooseEnum>("output_species_unit");

    uo_params.applyParameters(parameters());

    _problem->addUserObject(uo_type, uo_name, uo_params);
  }

#endif
}

void
ChemicalCompositionAction::readCSV()
{
  const auto & filename = getParam<FileName>("initial_values");
  std::ifstream file(filename.c_str());
  if (!file.good())
    paramError("initial_values", "Error opening file '", filename, "'.");

  std::string line;
  std::vector<std::string> items;

  // skip header
  std::getline(file, line);
  while (std::getline(file, line))
  {
    MooseUtils::tokenize(line, items, 1, ",");
    if (items.empty())
      continue;
    if (items.size() != 2)
      paramError("initial_value", "Unexpected line in CSV file: ", line);

    _initial_conditions[items[0]] = MooseUtils::convert<Real>(items[1]);
  }
}
