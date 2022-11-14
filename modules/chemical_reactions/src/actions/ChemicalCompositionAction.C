//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChemicalCompositionAction.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseUtils.h"

#include "libmesh/string_to_enum.h"

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#include "checkUnits.h"

registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_variable");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_aux_variable");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_ic");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_user_object");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_aux_kernel");
#endif

InputParameters
ChemicalCompositionAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Sets up the thermodynamic model and variables for the "
                             "thermochemistry using Thermochimica.");
  params.addParam<std::vector<std::string>>("elements", "List of chemical elements");
  params.addParam<FileName>("initial_values", "The CSV file name with initial conditions.");
  MooseEnum varType("BASE AUX", "AUX");
  params.addParam<MooseEnum>("var_type", varType, "Variable type to be generated");
  params.addParam<FileName>("thermofile", "Thermodynamics model file");

  MooseEnum tUnit("K C F R", "K");
  params.addParam<MooseEnum>("tunit", tUnit, "Temperature Unit");
  MooseEnum pUnit("atm psi bar Pa kPa", "atm");
  params.addParam<MooseEnum>("punit", pUnit, "Pressure Unit");
  MooseEnum mUnit(
      "mole_fraction atom_fraction atoms moles gram-atoms mass_fraction kilograms grams pounds",
      "moles");
  params.addParam<MooseEnum>("munit", mUnit, "Mass Unit");

  params.addParam<std::vector<std::string>>("output_phases", "List of phases to be output");
  params.addParam<std::vector<std::string>>(
      "output_species", "List species for which concentration in the phases is needed");
  params.addParam<std::vector<std::string>>(
      "element_potentials",
      "List of chemical elements for which chemical potentials are requested");
  return params;
}

ChemicalCompositionAction::ChemicalCompositionAction(const InputParameters & params)
  : Action(params),
    _elements(getParam<std::vector<std::string>>("elements")),
    _var_type(getParam<MooseEnum>("var_type").getEnum<VarType>()),
    _tunit(getParam<MooseEnum>("tunit")),
    _punit(getParam<MooseEnum>("punit")),
    _munit(getParam<MooseEnum>("munit")),
    _phases(getParam<std::vector<std::string>>("output_phases")),
    _species(getParam<std::vector<std::string>>("output_species")),
    _element_potentials(getParam<std::vector<std::string>>("element_potentials"))
{
  std::replace(_munit.begin(), _munit.end(), '_', ' ');
#ifndef THERMOCHIMICA_ENABLED
  mooseError("Thermochimica disabled");
#endif
}

void
ChemicalCompositionAction::act()
{
#ifdef THERMOCHIMICA_ENABLED
  //
  // Add nonlinear Variables
  //
  if (_current_task == "add_variable" && _var_type == VarType::Nonlinear)
  {
    const std::string var_type = "MooseVariable";
    auto params = _factory.getValidParams(var_type);
    const bool second = _problem->mesh().hasSecondOrderElements();
    params.set<MooseEnum>("order") = second ? "SECOND" : "FIRST";
    params.set<MooseEnum>("family") = "LAGRANGE";

    for (const auto i : index_range(_elements))
      _problem->addVariable(var_type, _elements[i], params);

    for (const auto i : index_range(_phases))
      _problem->addVariable(var_type, _phases[i], params);

    for (const auto i : index_range(_species))
      _problem->addVariable(var_type, _species[i], params);
  }

  //
  // Add AuxVariables
  //
  if (_current_task == "add_aux_variable" && _var_type == VarType::Aux)
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
  // Initiate Chemistry Model
  //
  if (_current_task == "add_user_object")
  {
    // initialize database in Thermochimica
    if (isParamValid("thermofile"))
    {
      const auto thermo_file = getParam<FileName>("thermofile");
      if (thermo_file.length() > 120)
        paramError("thermofile",
                   "Path exceeds thermiochimica's maximal permisible length of 120 with ",
                   thermo_file.length(),
                   " characters: ",
                   thermo_file);

      Thermochimica::setThermoFilename(thermo_file);

      // Read in thermodynamics model, only once
      Thermochimica::parseThermoFile();

      int idbg = Thermochimica::checkInfoThermo();
      if (idbg != 0)
        paramError("thermofile", "Thermochimica data file cannot be parsed. ", idbg);

      Thermochimica::checkTemperature(_tunit);
      Thermochimica::checkPressure(_punit);
      Thermochimica::checkMass(_munit);

      Thermochimica::setUnitTemperature(_tunit);
      Thermochimica::setUnitPressure(_punit);
      Thermochimica::setUnitMass(_munit);
    }
  }

  //
  // do we need those?!
  //
  if (_current_task == "add_aux_kernel" && _var_type == VarType::Aux)
  {
    auto params = _factory.getValidParams("SelfAux");
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};

    for (const auto i : index_range(_phases))
    {
      const std::string ker_name = _phases[i];
      params.set<AuxVariableName>("variable") = ker_name;
      _problem->addAuxKernel("SelfAux", ker_name, params);
    }

    for (const auto i : index_range(_species))
    {
      const std::string ker_name = _species[i];
      params.set<AuxVariableName>("variable") = ker_name;
      _problem->addAuxKernel("SelfAux", ker_name, params);
    }

    for (const auto i : index_range(_element_potentials))
    {
      const std::string ker_name = _element_potentials[i];
      params.set<AuxVariableName>("variable") = ker_name;
      _problem->addAuxKernel("SelfAux", ker_name, params);
    }
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

  unsigned int n_lines = 0;
  std::string line;
  while (getline(file, line))
  {
    // Replace all commas with spaces
    while (size_t pos = line.find(','))
    {
      if (pos == line.npos)
        break;
      line.replace(pos, 1, 1, ' ');
    }

    if (n_lines > 0)
    {
      std::istringstream iss(line);

      std::string el_name;
      iss >> el_name;
      Real f;
      iss >> f;

      _initial_conditions[el_name] = f;
    }
    n_lines++;
  }
}
