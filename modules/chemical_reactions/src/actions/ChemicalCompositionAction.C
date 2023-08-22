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
#include "MooseUtils.h"

#include "libmesh/string_to_enum.h"

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#include "checkUnits.h"
#endif

std::string ChemicalCompositionAction::_database_file = "";
bool ChemicalCompositionAction::_database_parsed = false;

registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_variable");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_aux_variable");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_ic");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_user_object");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_aux_kernel");

InputParameters
ChemicalCompositionAction::validParams()
{
  InputParameters params = Action::validParams();

  ThermochimicaUtils::addClassDescription(params,
                                          "Sets up the thermodynamic model and variables for the "
                                          "thermochemistry solve using Thermochimica.");

  params.addRequiredParam<std::vector<std::string>>("elements",
                                                    "List of chemical elements (or ALL)");
  params.addRequiredCoupledVar("temperature", "Name of temperature variable");
  params.addCoupledVar("pressure", "Name of pressure variable");
  MooseEnum reinit_type("none time nodal", "nodal");
  params.addParam<MooseEnum>(
      "reinitialization_type", reinit_type, "Reinitialization scheme to use with Thermochimica");
  params.addParam<FileName>("initial_values", "The CSV file name with initial conditions.");
  params.addParam<FileName>("thermofile", "Thermodynamics model file");

  MooseEnum tUnit("K C F R");
  params.addRequiredParam<MooseEnum>("tunit", tUnit, "Temperature Unit");
  MooseEnum pUnit("atm psi bar Pa kPa");
  params.addRequiredParam<MooseEnum>("punit", pUnit, "Pressure Unit");
  MooseEnum mUnit(
      "mole_fraction atom_fraction atoms moles gram-atoms mass_fraction kilograms grams pounds");
  params.addRequiredParam<MooseEnum>("munit", mUnit, "Mass Unit");
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.addParam<ExecFlagEnum>(
      "execute_on", exec_enum, "When to execute the ThermochimicaNodalData UO");

  params.addParam<std::vector<std::string>>("output_phases", "List of phases to be output");
  params.addParam<std::vector<std::string>>(
      "output_species", "List species for which concentration in the phases is needed");
  MooseEnum mUnit_op("moles mole_fraction", "moles");
  params.addParam<MooseEnum>(
      "output_species_unit", mUnit_op, "Mass unit for output species: mole_fractions or moles");
  params.addParam<std::vector<std::string>>(
      "output_element_potentials",
      "List of chemical elements for which chemical potentials are requested");
  params.addParam<std::vector<std::string>>(
      "output_vapor_pressures",
      "List of gas phase species for which vapor pressures are requested");
  params.addParam<std::vector<std::string>>(
      "output_element_phases",
      "List of elements whose molar amounts in specific phases are requested");
  params.addParam<std::string>(
      "uo_name", "Thermochimica", "Name of the ThermochimicaNodalDataUserObject.");
  return params;
}

ChemicalCompositionAction::ChemicalCompositionAction(const InputParameters & params)
  : Action(params),
    _elements(getParam<std::vector<std::string>>("elements")),
    _tunit(getParam<MooseEnum>("tunit")),
    _punit(getParam<MooseEnum>("punit")),
    _munit(getParam<MooseEnum>("munit")),
    _phases(getParam<std::vector<std::string>>("output_phases")),
    _species(getParam<std::vector<std::string>>("output_species")),
    _output_mass_unit(getParam<MooseEnum>("output_species_unit")),
    _element_potentials(getParam<std::vector<std::string>>("output_element_potentials")),
    _vapor_pressures(getParam<std::vector<std::string>>("output_vapor_pressures")),
    _element_phases(getParam<std::vector<std::string>>("output_element_phases")),
    _reinit(getParam<MooseEnum>("reinitialization_type")),
    _uo_name(getParam<std::string>("uo_name"))
{
  ThermochimicaUtils::checkLibraryAvailability(*this);

  std::replace(_munit.begin(), _munit.end(), '_', ' ');

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

    if (!_database_parsed)
    {
      Thermochimica::setThermoFilename(thermo_file);

      // Read in thermodynamics model, only once
      Thermochimica::parseThermoFile();

      const auto idbg = Thermochimica::checkInfoThermo();
      if (idbg != 0)
        paramError("thermofile", "Thermochimica data file cannot be parsed. ", idbg);
      else
      {
        _database_file = thermo_file;
        _database_parsed = true;
      }
    }
    else if (_database_parsed && thermo_file != _database_file)
      paramError("thermofile",
                 "Thermodynamic database ",
                 _database_file,
                 " already parsed. Cannot parse database ",
                 thermo_file);
  }

  // Set thermochimica units
  Thermochimica::checkTemperature(_tunit);
  Thermochimica::setUnitTemperature(_tunit);
  int idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
    paramError("tunit", "Cannot set temperature unit in Thermochimica", idbg);

  Thermochimica::checkPressure(_punit);
  Thermochimica::setUnitPressure(_punit);
  idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
    paramError("punit", "Cannot set pressure unit in Thermochimica", idbg);

  Thermochimica::checkMass(_munit);
  Thermochimica::setUnitMass(_munit);
  idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
    paramError("munit", "Cannot set mass unit in Thermochimica", idbg);

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
    auto phases = Thermochimica::getPhaseNamesSystem();
    auto n_db_species = Thermochimica::getNumberSpeciesSystem();
    auto species = Thermochimica::getSpeciesSystem();
    for (auto i : index_range(n_db_species))
      if (Thermochimica::isPhaseMQM(i))
      {
        auto [pairs, quads, idbg] =
            Thermochimica::getMqmqaNumberPairsQuads(Thermochimica::getPhaseNamesSystem()[i]);
        n_db_species[i] = pairs;
      }

    if (_species.size() == 1 && _species[0] == "ALL")
    {
      if (!n_db_species.empty())
        _species.resize(n_db_species.back());
      else
        mooseInfo("ChemicalCompositionAction species: 'ALL' specified in input file. Thermochimica "
                  "returned no possible species.");

      _species.clear();
      _tokenized_species.clear();
      for (const auto i : index_range(species))
        for (const auto j : index_range(species[i]))
        {
          _species.push_back(phases[i] + ":" + species[i][j]);
          _tokenized_species.push_back(std::make_pair(phases[i], species[i][j]));
        }
      mooseInfo("ChemicalCompositionAction species: 'ALL' specified in input file. Using: ",
                Moose::stringify(_species));
    }
    else
      for (const auto i : index_range(_species))
      {
        _tokenized_species.resize(_species.size());
        std::vector<std::string> tokens;
        MooseUtils::tokenize(_species[i], tokens, 1, ":");
        if (tokens.size() == 1)
          paramError("output_species", "No ':' separator found in variable '", _species[i], "'");

        auto phase_index = std::find(phases.begin(), phases.end(), tokens[0]);
        if (phase_index == phases.end())
          paramError("output_species",
                     "Phase '",
                     tokens[0],
                     "' of output species '",
                     _species[i],
                     "' not found in the simulation.");
        auto sp = species[std::distance(phases.begin(), phase_index)];
        if (std::find(sp.begin(), sp.end(), tokens[1]) == sp.end())
          paramError(
              "output_species", "Species '", tokens[1], "' was not found in the simulation.");
        _tokenized_species[i] = std::make_pair(tokens[0], tokens[1]);
      }
  }

  if (isParamValid("output_element_potentials"))
  {
    if (_element_potentials.size() == 1 && _element_potentials[0] == "ALL")
    {
      _tokenized_element_potentials.resize(_elements.size());
      _tokenized_element_potentials = _elements;
      _element_potentials.resize(_elements.size());
      for (const auto i : index_range(_elements))
        _element_potentials[i] = "mu:" + _elements[i];
      mooseInfo(
          "ChemicalCompositionAction element potentials: 'ALL' specified in input file. Using: ",
          Moose::stringify(_element_potentials));
    }
    else
    {
      _tokenized_element_potentials.resize(_element_potentials.size());
      for (const auto i : index_range(_element_potentials))
      {
        std::vector<std::string> tokens;
        MooseUtils::tokenize(_element_potentials[i], tokens, 1, ":");
        if (tokens.size() == 1)
          paramError("output_element_potentials",
                     "No ':' separator found in variable '",
                     _element_potentials[i],
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
    if (!Thermochimica::isPhaseGas(0))
      paramError("output_vapor_pressures",
                 "No gas phase found in the simulation. Cannot output vapor pressures.");
    if (_vapor_pressures.size() == 1 && _vapor_pressures[0] == "ALL")
    {
      _vapor_pressures.resize(Thermochimica::getNumberSpeciesSystem()[0]);
      _tokenized_vapor_species.resize(Thermochimica::getNumberSpeciesSystem()[0]);
      auto db_gas_species = Thermochimica::getSpeciesInPhase(0);
      auto gas_name = Thermochimica::getPhaseNamesSystem()[0];
      for (const auto i : index_range(db_gas_species))
      {
        _vapor_pressures[i] = "vp:" + gas_name + ':' + db_gas_species[i];
        _tokenized_vapor_species[i] = std::make_pair(gas_name, db_gas_species[i]);
      }
      mooseInfo("ChemicalCompositionAction vapor pressures: 'ALL' specified in input file. Using: ",
                Moose::stringify(_vapor_pressures));
    }
    else
    {
      auto db_gas_species = Thermochimica::getSpeciesInPhase(0);
      _tokenized_vapor_species.resize(_vapor_pressures.size());
      for (const auto i : index_range(_vapor_pressures))
      {
        std::vector<std::string> tokens;
        MooseUtils::tokenize(_vapor_pressures[i], tokens, 1, ":");
        if (tokens.size() == 1)
          paramError("output_vapor_pressures",
                     "No ':' separator found in variable '",
                     _vapor_pressures[i],
                     "'");
        if (tokens[1] != Thermochimica::getPhaseNamesSystem()[0])
          paramError("output_vapor_pressures",
                     "Phase '",
                     tokens[1],
                     "' of vapor species '",
                     _vapor_pressures[i],
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
    auto phases = Thermochimica::getPhaseNamesSystem();
    if (_element_phases.size() == 1 && _element_phases[0] == "ALL")
    {
      _element_phases.resize(_elements.size() * phases.size());
      _tokenized_phase_elements.resize(_elements.size() * phases.size());
      for (const auto i : index_range(phases))
        for (const auto j : index_range(_elements))
        {
          _element_phases[i * _elements.size() + j] = phases[i] + ':' + _elements[j];
          _tokenized_phase_elements[i * _elements.size() + j] =
              std::make_pair(phases[i], _elements[j]);
        }
      mooseInfo(
          "ChemicalCompositionAction elements in phase: 'ALL' specified in input file. Using: ",
          Moose::stringify(_element_phases));
    }
    else
    {
      _tokenized_phase_elements.resize(_element_phases.size());
      for (const auto i : index_range(_element_phases))
      {
        std::vector<std::string> tokens;
        MooseUtils::tokenize(_element_phases[i], tokens, 1, ":");
        if (tokens.size() == 1)
          paramError("output_element_phases",
                     "No ':' separator found in variable '",
                     _element_phases[i],
                     "'");
        if (std::find(phases.begin(), phases.end(), tokens[1]) == phases.end())
          paramError("output_element_phases",
                     "Phase '",
                     tokens[1],
                     "' of '",
                     _element_phases[i],
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
    const std::string aux_var_type = "MooseVariable";
    auto params = _factory.getValidParams(aux_var_type);
    const bool second = _problem->mesh().hasSecondOrderElements();
    params.set<MooseEnum>("order") = second ? "SECOND" : "FIRST";
    params.set<MooseEnum>("family") = "LAGRANGE";

    for (const auto i : index_range(_elements))
      _problem->addAuxVariable(aux_var_type, _elements[i], params);

    for (const auto i : index_range(_phases))
      _problem->addAuxVariable(aux_var_type, _phases[i], params);

    for (const auto i : index_range(_species))
      _problem->addAuxVariable(aux_var_type, _species[i], params);

    for (const auto i : index_range(_element_potentials))
      _problem->addAuxVariable(aux_var_type, _element_potentials[i], params);

    for (const auto i : index_range(_vapor_pressures))
      _problem->addAuxVariable(aux_var_type, _vapor_pressures[i], params);

    for (const auto i : index_range(_element_phases))
      _problem->addAuxVariable(aux_var_type, _element_phases[i], params);
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

    auto uo_params = _factory.getValidParams("ThermochimicaNodalData");

    std::copy(_elements.begin(),
              _elements.end(),
              std::back_inserter(uo_params.set<std::vector<VariableName>>("elements")));

    if (isParamValid("output_phases"))
      std::copy(_phases.begin(),
                _phases.end(),
                std::back_inserter(uo_params.set<std::vector<VariableName>>("output_phases")));

    if (isParamValid("output_species"))
      uo_params.set<std::vector<VariableName>>("output_species")
          .insert(uo_params.set<std::vector<VariableName>>("output_species").end(),
                  _species.begin(),
                  _species.end());

    if (isParamValid("output_element_potentials"))
      uo_params.set<std::vector<VariableName>>("output_element_potentials")
          .insert(uo_params.set<std::vector<VariableName>>("output_element_potentials").end(),
                  _element_potentials.begin(),
                  _element_potentials.end());

    if (isParamValid("output_vapor_pressures"))
      uo_params.set<std::vector<VariableName>>("output_vapor_pressures")
          .insert(uo_params.set<std::vector<VariableName>>("output_vapor_pressures").end(),
                  _vapor_pressures.begin(),
                  _vapor_pressures.end());

    if (isParamValid("output_element_phases"))
      uo_params.set<std::vector<VariableName>>("output_element_phases")
          .insert(uo_params.set<std::vector<VariableName>>("output_element_phases").end(),
                  _element_phases.begin(),
                  _element_phases.end());

    uo_params.set<ChemicalCompositionAction *>("_chemical_composition_action") = this;

    uo_params.applyParameters(parameters());

    _problem->addUserObject("ThermochimicaNodalData", _uo_name, uo_params);
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
