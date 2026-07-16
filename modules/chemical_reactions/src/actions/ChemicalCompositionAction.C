//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChemicalCompositionAction.h"
#include "CommonChemicalCompositionAction.h"
#include "ThermochimicaUtils.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "AddVariableAction.h"
#include "BlockRestrictable.h"
#include "InputParameterWarehouse.h"
#include "ActionWarehouse.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <set>
#include <type_traits>

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#include "checkUnits.h"
#endif

registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_variable");
registerMooseAction("ChemicalReactionsApp",
                    ChemicalCompositionAction,
                    "setup_chemical_composition");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_aux_variable");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_ic");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_user_object");
registerMooseAction("ChemicalReactionsApp", ChemicalCompositionAction, "add_aux_kernel");

namespace
{
std::vector<std::string>
tokens(const std::string & value)
{
  std::vector<std::string> result;
  MooseUtils::tokenize(value, result, 1, ":");
  return result;
}

std::string
objectName(std::string action_name)
{
  std::replace_if(
      action_name.begin(),
      action_name.end(),
      [](const char c) { return c == '/' || c == ':'; },
      '_');
  return "Thermochimica_" + action_name;
}
}

InputParameters
ChemicalCompositionAction::validParams()
{
  InputParameters params = Action::validParams();
  params += BlockRestrictable::validParams();

  ThermochimicaUtils::addClassDescription(
      params, "Creates variables and an exact batched Thermochimica equilibrium executor.");

  params.addParam<std::vector<std::string>>(
      "elements", {"ALL"}, "Chemical elements to include, or ALL");
  params.addCoupledVar("temperature", "Temperature variable or constant");
  params.addCoupledVar("pressure", 1.0, "Pressure variable or constant");
  params.addParam<FileName>("thermodynamic_database", "Thermochimica database file");

  params.addParam<MooseEnum>("temperature_unit", MooseEnum("K C F R"), "Temperature unit");
  params.addParam<MooseEnum>("pressure_unit", MooseEnum("atm psi bar Pa kPa"), "Pressure unit");
  params.addParam<MooseEnum>("composition_unit",
                             MooseEnum("mole_fraction atom_fraction atoms moles gram-atoms "
                                       "mass_fraction kilograms grams pounds"),
                             "Element composition unit");

  params.addParam<MooseEnum>("evaluation_location",
                             MooseEnum("nodal elemental", "nodal"),
                             "Location at which equilibrium is evaluated");
  params.addParam<MooseEnum>("warm_start",
                             MooseEnum("previous_solve previous_timestep none", "previous_solve"),
                             "Exact Thermochimica warm-start strategy");
  params.addRangeCheckedParam<unsigned int>(
      "batch_size", 32, "batch_size > 0", "Number of states sent to each worker request");
  params.addParam<bool>(
      "report_performance", false, "Report state, batch, warm-start, and worker solve counters");
  params.addParam<FileName>("initial_composition_file", "CSV file containing constant element ICs");

  ExecFlagEnum execute_on = MooseUtils::getDefaultExecFlagEnum();
  execute_on = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.addParam<ExecFlagEnum>("execute_on", execute_on, "When to evaluate equilibrium");

  params.addParam<std::vector<std::string>>("output_phases", {}, "Phases to output, or ALL");
  params.addParam<std::vector<std::string>>(
      "output_species", {}, "Species to output as phase:species, or ALL");
  params.addParam<MooseEnum>("species_output_unit",
                             MooseEnum("moles mole_fraction", "moles"),
                             "Unit used for species outputs");
  params.addParam<std::vector<std::string>>(
      "output_element_potentials", {}, "Elements whose chemical potentials are output, or ALL");
  params.addParam<std::vector<std::string>>(
      "output_vapor_pressures", {}, "Vapor species as gas_phase:species, or ALL");
  params.addParam<std::vector<std::string>>(
      "output_element_phases", {}, "Element amounts as phase:element, or ALL");
  return params;
}

ChemicalCompositionAction::ChemicalCompositionAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
ChemicalCompositionAction::initializeConfiguration()
{
  mooseAssert(!_configuration, "Chemical composition configuration was initialized twice");
  _configuration = std::make_shared<ThermochimicaConfiguration>();

  const auto & warehouse_params = _app.getInputParameterWarehouse().getInputParameters();
  InputParameters & pars(*(warehouse_params.find(uniqueActionName())->second.get()));

  const auto common = _awh.getActions<CommonChemicalCompositionAction>();
  if (common.size() == 1)
    pars.applyParameters(common[0]->parameters());

  for (const auto & parameter : {"thermodynamic_database",
                                 "temperature_unit",
                                 "pressure_unit",
                                 "composition_unit",
                                 "temperature"})
    if (!isParamValid(parameter))
      paramError(parameter, "This parameter is required after applying common parameters.");

  ThermochimicaUtils::checkLibraryAvailability(*this);

  auto & config = *_configuration;
  config.database = getParam<FileName>("thermodynamic_database");
  config.temperature_unit = std::string(getParam<MooseEnum>("temperature_unit"));
  config.pressure_unit = std::string(getParam<MooseEnum>("pressure_unit"));
  config.composition_unit = std::string(getParam<MooseEnum>("composition_unit"));
  std::replace(config.composition_unit.begin(), config.composition_unit.end(), '_', ' ');
  auto coupledInput = [this](const std::string & parameter)
  {
    if (this->parameters().hasDefaultCoupledValue(parameter))
      return Moose::stringify(this->parameters().defaultCoupledValue(parameter));
    const auto & variables = getParam<std::vector<VariableName>>(parameter);
    return std::string(variables.front());
  };
  config.temperature = coupledInput("temperature");
  config.pressure = coupledInput("pressure");
  config.batch_size = getParam<unsigned int>("batch_size");
  config.report_performance = getParam<bool>("report_performance");

  const auto location = getParam<MooseEnum>("evaluation_location");
  config.location = location == "nodal" ? ThermochimicaConfiguration::EvaluationLocation::NODAL
                                        : ThermochimicaConfiguration::EvaluationLocation::ELEMENTAL;
  const auto warm_start = getParam<MooseEnum>("warm_start");
  if (warm_start == "previous_solve")
    config.warm_start = ThermochimicaConfiguration::WarmStart::PREVIOUS_SOLVE;
  else if (warm_start == "previous_timestep")
    config.warm_start = ThermochimicaConfiguration::WarmStart::PREVIOUS_TIMESTEP;
  else
    config.warm_start = ThermochimicaConfiguration::WarmStart::NONE;
#ifdef THERMOCHIMICA_ENABLED
  if (config.database.length() > 1024)
    paramError("thermodynamic_database",
               "Path exceeds Thermochimica's maximum length of 1024 characters: ",
               config.database);

  Thermochimica::setThermoFilename(config.database);
  Thermochimica::parseThermoFile();
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
    paramError("thermodynamic_database", "Thermochimica data file cannot be parsed. ", info);

  Thermochimica::checkTemperature(config.temperature_unit);
  Thermochimica::setUnitTemperature(config.temperature_unit);
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
    paramError("temperature_unit", "Cannot set Thermochimica temperature unit. ", info);
  Thermochimica::checkPressure(config.pressure_unit);
  Thermochimica::setUnitPressure(config.pressure_unit);
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
    paramError("pressure_unit", "Cannot set Thermochimica pressure unit. ", info);
  Thermochimica::checkMass(config.composition_unit);
  Thermochimica::setUnitMass(config.composition_unit);
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
    paramError("composition_unit", "Cannot set Thermochimica composition unit. ", info);

  config.elements = getParam<std::vector<std::string>>("elements");
  const auto database_elements = Thermochimica::getElementsDatabase();
  if (config.elements.size() == 1 && config.elements[0] == "ALL")
    config.elements = database_elements;
  else
    for (const auto & element : config.elements)
      if (std::find(database_elements.begin(), database_elements.end(), element) ==
          database_elements.end())
        paramError("elements", "Element '", element, "' was not found in the database.");
  if (config.elements.empty())
    paramError("elements", "At least one element must be configured.");

  config.element_ids.reserve(config.elements.size());
  for (const auto & element : config.elements)
  {
    config.element_ids.push_back(Thermochimica::atomicNumber(element));
    config.element_variables.push_back(element);
  }

  Thermochimica::setTemperaturePressure(1000.0, 1.0);
  Thermochimica::setElementMass(0, 0.0);
  for (const auto element_id : config.element_ids)
    Thermochimica::setElementMass(element_id, 1.0);
  Thermochimica::setup();

  const auto database_phases = Thermochimica::getPhaseNamesSystem();
  const auto database_species = Thermochimica::getSpeciesSystem();
  std::set<std::string> unique_phases;
  for (const auto phase_index : index_range(database_phases))
    if (unique_phases.insert(database_phases[phase_index]).second)
    {
      config.phase_names.push_back(database_phases[phase_index]);
      config.phase_indices.push_back(static_cast<int>(phase_index));
    }

  for (const auto & name : config.element_variables)
    if (!_variable_origins.emplace(name, "elements").second)
      paramError("elements", "Duplicate generated variable name '", name, "'.");

  buildLegacyOutputDescriptors(database_phases, database_species);
  buildTypedOutputDescriptors(database_phases, database_species);

  Thermochimica::resetThermoAll();
#endif
}

#ifdef THERMOCHIMICA_ENABLED
void
ChemicalCompositionAction::addOutputDescriptor(ThermochimicaConfiguration::OutputDescriptor output,
                                               const InputParameters & source,
                                               const std::string & origin,
                                               const std::string & variable_parameter)
{
  const auto & name = std::visit(
      [](const auto & descriptor) -> const VariableName & { return descriptor.variable; }, output);
  const auto [it, inserted] = _variable_origins.emplace(name, origin);
  if (!inserted)
    source.paramError(variable_parameter,
                      "Duplicate generated variable name '",
                      name,
                      "'. It was first generated by '",
                      it->second,
                      "'.");
  _configuration->outputs.push_back(std::move(output));
}

int
ChemicalCompositionAction::phaseSystemIndex(const std::string & phase,
                                            const InputParameters & source,
                                            const std::string & parameter,
                                            const std::vector<std::string> & database_phases) const
{
  const auto it = std::find(database_phases.begin(), database_phases.end(), phase);
  if (it == database_phases.end())
    source.paramError(parameter, "Phase '", phase, "' was not found in the configured system.");
  return static_cast<int>(std::distance(database_phases.begin(), it));
}

int
ChemicalCompositionAction::speciesPhaseIndex(
    const int phase_index,
    const std::string & species,
    const std::vector<std::vector<std::string>> & database_species) const
{
  const auto it = std::find(
      database_species[phase_index].begin(), database_species[phase_index].end(), species);
  return static_cast<int>(std::distance(database_species[phase_index].begin(), it));
}

void
ChemicalCompositionAction::addOutputRequest(const ThermochimicaPhaseRequest & request,
                                            const InputParameters & source,
                                            const std::string & origin,
                                            const std::string & phase_parameter,
                                            const std::vector<std::string> & database_phases)
{
  addOutputDescriptor(
      ThermochimicaConfiguration::PhaseOutput{
          request.variable,
          request.phase,
          phaseSystemIndex(request.phase, source, phase_parameter, database_phases),
          request.unit},
      source,
      origin,
      origin == phase_parameter ? origin : "variable");
  if (request.unit == ThermochimicaConfiguration::AmountUnit::MOLE_FRACTION)
    _configuration->needs_phase_total = true;
}

void
ChemicalCompositionAction::addOutputRequest(
    const ThermochimicaSpeciesRequest & request,
    const InputParameters & source,
    const std::string & origin,
    const std::string & phase_parameter,
    const std::string & species_parameter,
    const std::vector<std::string> & database_phases,
    const std::vector<std::vector<std::string>> & database_species)
{
  const auto phase_index =
      phaseSystemIndex(request.phase, source, phase_parameter, database_phases);
  if (phase_index >= static_cast<int>(database_species.size()))
    source.paramError(
        phase_parameter, "Phase '", request.phase, "' does not contain solution species.");
  if (std::find(database_species[phase_index].begin(),
                database_species[phase_index].end(),
                request.species) == database_species[phase_index].end())
    source.paramError(species_parameter,
                      "Species '",
                      request.species,
                      "' was not found in phase '",
                      request.phase,
                      "'.");
  const bool is_mqm = Thermochimica::isPhaseMQM(phase_index);
  addOutputDescriptor(
      ThermochimicaConfiguration::SpeciesOutput{
          request.variable,
          request.phase,
          request.species,
          phase_index,
          is_mqm ? -1 : speciesPhaseIndex(phase_index, request.species, database_species),
          is_mqm,
          request.unit},
      source,
      origin,
      origin == species_parameter ? origin : "variable");
}

void
ChemicalCompositionAction::addOutputRequest(const ThermochimicaElementPotentialRequest & request,
                                            const InputParameters & source,
                                            const std::string & origin,
                                            const std::string & element_parameter)
{
  if (std::find(_configuration->elements.begin(),
                _configuration->elements.end(),
                request.element) == _configuration->elements.end())
    source.paramError(element_parameter, "Element '", request.element, "' is not configured.");
  const auto [element_index, info] =
      Thermochimica::getElementIndex(Thermochimica::atomicNumber(request.element));
  if (info != 0)
    source.paramError(element_parameter,
                      "Element '",
                      request.element,
                      "' could not be resolved in the configured system.");
  addOutputDescriptor(ThermochimicaConfiguration::ElementPotentialOutput{request.variable,
                                                                         request.element,
                                                                         element_index},
                      source,
                      origin,
                      origin == element_parameter ? origin : "variable");
}

void
ChemicalCompositionAction::addOutputRequest(
    const ThermochimicaVaporPressureRequest & request,
    const InputParameters & source,
    const std::string & origin,
    const std::string & phase_parameter,
    const std::string & species_parameter,
    const std::vector<std::string> & database_phases,
    const std::vector<std::vector<std::string>> & database_species)
{
  const auto phase_index =
      phaseSystemIndex(request.phase, source, phase_parameter, database_phases);
  if (!Thermochimica::isPhaseGas(phase_index))
    source.paramError(phase_parameter, "Phase '", request.phase, "' is not a gas phase.");
  if (std::find(database_species[phase_index].begin(),
                database_species[phase_index].end(),
                request.species) == database_species[phase_index].end())
    source.paramError(species_parameter,
                      "Species '",
                      request.species,
                      "' was not found in gas phase '",
                      request.phase,
                      "'.");
  addOutputDescriptor(
      ThermochimicaConfiguration::VaporPressureOutput{
          request.variable,
          request.phase,
          request.species,
          phase_index,
          speciesPhaseIndex(phase_index, request.species, database_species)},
      source,
      origin,
      origin == species_parameter ? origin : "variable");
}

void
ChemicalCompositionAction::addOutputRequest(const ThermochimicaElementDistributionRequest & request,
                                            const InputParameters & source,
                                            const std::string & origin,
                                            const std::string & phase_parameter,
                                            const std::string & element_parameter,
                                            const std::vector<std::string> & database_phases)
{
  const auto phase_index =
      phaseSystemIndex(request.phase, source, phase_parameter, database_phases);
  if (std::find(_configuration->elements.begin(),
                _configuration->elements.end(),
                request.element) == _configuration->elements.end())
    source.paramError(element_parameter, "Element '", request.element, "' is not configured.");
  const auto [element_index, info] =
      Thermochimica::getElementIndex(Thermochimica::atomicNumber(request.element));
  if (info != 0)
    source.paramError(element_parameter,
                      "Element '",
                      request.element,
                      "' could not be resolved in the configured system.");
  addOutputDescriptor(ThermochimicaConfiguration::ElementDistributionOutput{request.variable,
                                                                            request.phase,
                                                                            request.element,
                                                                            phase_index,
                                                                            element_index,
                                                                            request.unit},
                      source,
                      origin,
                      origin == element_parameter ? origin : "variable");
}

void
ChemicalCompositionAction::buildLegacyOutputDescriptors(
    const std::vector<std::string> & database_phases,
    const std::vector<std::vector<std::string>> & database_species)
{
  auto & config = *_configuration;
  const auto species_unit = getParam<MooseEnum>("species_output_unit") == "moles"
                                ? ThermochimicaConfiguration::AmountUnit::MOLES
                                : ThermochimicaConfiguration::AmountUnit::MOLE_FRACTION;

  const std::array legacy_parameters{"output_phases",
                                     "output_species",
                                     "output_element_potentials",
                                     "output_vapor_pressures",
                                     "output_element_phases"};
  if (std::any_of(legacy_parameters.begin(),
                  legacy_parameters.end(),
                  [&](const auto parameter) { return isParamSetByUser(parameter); }))
    mooseDeprecated("The flat ChemicalComposition output parameters are deprecated. Define typed "
                    "output blocks under [Outputs] in each thermodynamic system instead.");

  auto phases = getParam<std::vector<std::string>>("output_phases");
  if (phases.size() == 1 && phases[0] == "ALL")
  {
    phases.clear();
    std::set<std::string> expanded;
    for (const auto & phase : database_phases)
      if (expanded.insert(phase).second)
        phases.push_back(phase);
  }
  for (const auto & phase : phases)
    addOutputRequest(
        ThermochimicaPhaseRequest{phase, phase, ThermochimicaConfiguration::AmountUnit::MOLES},
        parameters(),
        "output_phases",
        "output_phases",
        database_phases);

  auto species = getParam<std::vector<std::string>>("output_species");
  if (species.size() == 1 && species[0] == "ALL")
  {
    species.clear();
    std::set<std::string> expanded;
    for (const auto phase_index : index_range(database_species))
      for (const auto & name : database_species[phase_index])
      {
        const auto selector = database_phases[phase_index] + ":" + name;
        if (expanded.insert(selector).second)
          species.push_back(selector);
      }
  }
  for (const auto & value : species)
  {
    const auto split = tokens(value);
    if (split.size() != 2)
      paramError("output_species", "Expected 'phase:species', received '", value, "'.");
    addOutputRequest(ThermochimicaSpeciesRequest{value, split[0], split[1], species_unit},
                     parameters(),
                     "output_species",
                     "output_species",
                     "output_species",
                     database_phases,
                     database_species);
  }

  auto potentials = getParam<std::vector<std::string>>("output_element_potentials");
  if (potentials.size() == 1 && potentials[0] == "ALL")
    potentials = config.elements;
  for (const auto & element : potentials)
    addOutputRequest(ThermochimicaElementPotentialRequest{"mu:" + element, element},
                     parameters(),
                     "output_element_potentials",
                     "output_element_potentials");

  auto vapors = getParam<std::vector<std::string>>("output_vapor_pressures");
  if (vapors.size() == 1 && vapors[0] == "ALL")
  {
    vapors.clear();
    for (const auto phase_index : index_range(database_phases))
      if (Thermochimica::isPhaseGas(phase_index))
        for (const auto & name : database_species[phase_index])
          vapors.push_back(database_phases[phase_index] + ":" + name);
  }
  for (const auto & value : vapors)
  {
    const auto split = tokens(value);
    if (split.size() != 2)
      paramError("output_vapor_pressures", "Expected 'gas_phase:species', received '", value, "'.");
    addOutputRequest(ThermochimicaVaporPressureRequest{"vp:" + value, split[0], split[1]},
                     parameters(),
                     "output_vapor_pressures",
                     "output_vapor_pressures",
                     "output_vapor_pressures",
                     database_phases,
                     database_species);
  }

  auto phase_elements = getParam<std::vector<std::string>>("output_element_phases");
  if (phase_elements.size() == 1 && phase_elements[0] == "ALL")
  {
    phase_elements.clear();
    std::set<std::string> expanded;
    for (const auto & phase : database_phases)
      for (const auto & element : config.elements)
      {
        const auto selector = phase + ":" + element;
        if (expanded.insert(selector).second)
          phase_elements.push_back(selector);
      }
  }
  for (const auto & value : phase_elements)
  {
    const auto split = tokens(value);
    if (split.size() != 2)
      paramError("output_element_phases", "Expected 'phase:element', received '", value, "'.");
    addOutputRequest(
        ThermochimicaElementDistributionRequest{
            "ep:" + value, split[0], split[1], ThermochimicaConfiguration::DistributionUnit::MOLES},
        parameters(),
        "output_element_phases",
        "output_element_phases",
        "output_element_phases",
        database_phases);
  }
}

void
ChemicalCompositionAction::buildTypedOutputDescriptors(
    const std::vector<std::string> & database_phases,
    const std::vector<std::vector<std::string>> & database_species)
{
  std::vector<const ThermochimicaOutputAction *> output_actions;
  for (const auto & action : _awh.allActionBlocks())
    if (const auto output = dynamic_cast<const ThermochimicaOutputAction *>(action.get());
        output && output->parentPath() == parameters().blockFullpath())
      output_actions.push_back(output);

  std::sort(output_actions.begin(),
            output_actions.end(),
            [](const auto * left, const auto * right) { return left->origin() < right->origin(); });

  for (const auto * action : output_actions)
    std::visit(
        [&](const auto & request)
        {
          using Request = std::decay_t<decltype(request)>;
          if constexpr (std::is_same_v<Request, ThermochimicaPhaseRequest>)
            addOutputRequest(
                request, action->parameters(), action->origin(), "phase", database_phases);
          else if constexpr (std::is_same_v<Request, ThermochimicaSpeciesRequest>)
            addOutputRequest(request,
                             action->parameters(),
                             action->origin(),
                             "phase",
                             "species",
                             database_phases,
                             database_species);
          else if constexpr (std::is_same_v<Request, ThermochimicaElementPotentialRequest>)
            addOutputRequest(request, action->parameters(), action->origin(), "element");
          else if constexpr (std::is_same_v<Request, ThermochimicaVaporPressureRequest>)
            addOutputRequest(request,
                             action->parameters(),
                             action->origin(),
                             "phase",
                             "species",
                             database_phases,
                             database_species);
          else if constexpr (std::is_same_v<Request, ThermochimicaElementDistributionRequest>)
            addOutputRequest(request,
                             action->parameters(),
                             action->origin(),
                             "phase",
                             "element",
                             database_phases);
        },
        action->request());
}
#endif

void
ChemicalCompositionAction::act()
{
#ifdef THERMOCHIMICA_ENABLED
  if (_current_task == "setup_chemical_composition")
  {
    initializeConfiguration();
    return;
  }

  if (!_configuration)
    mooseError("Chemical composition configuration has not been initialized before task '",
               _current_task,
               ".");
  const auto elemental =
      _configuration->location == ThermochimicaConfiguration::EvaluationLocation::ELEMENTAL;

  if (_current_task == "add_aux_variable")
  {
    const auto order = elemental                                   ? CONSTANT
                       : _problem->mesh().hasSecondOrderElements() ? SECOND
                                                                   : FIRST;
    const auto family = elemental ? MONOMIAL : LAGRANGE;
    const auto aux_var_type =
        AddVariableAction::variableType(FEType(order, family), elemental, false);
    auto params = _factory.getValidParams(aux_var_type);
    params.applySpecificParameters(parameters(), {"block"});
    for (const auto & name : _configuration->element_variables)
      _problem->addAuxVariable(aux_var_type, name, params);
    for (const auto & output : _configuration->outputs)
      std::visit([&](const auto & descriptor)
                 { _problem->addAuxVariable(aux_var_type, descriptor.variable, params); },
                 output);
  }

  if (_current_task == "add_ic" && isParamValid("initial_composition_file"))
  {
    readCSV();
    for (const auto & [variable, value] : _initial_conditions)
    {
      auto params = _factory.getValidParams("ConstantIC");
      params.set<VariableName>("variable") = variable;
      params.set<Real>("value") = value;
      params.applySpecificParameters(parameters(), {"block"});
      _problem->addInitialCondition("ConstantIC", variable + "_ic", params);
    }
  }

  if (_current_task == "add_user_object")
  {
    auto params = _factory.getValidParams("ThermochimicaData");
    params.set<ThermochimicaConfigurationPtr>("_configuration") = _configuration;
    params.set<ExecFlagEnum>("execute_on") = getParam<ExecFlagEnum>("execute_on");
    params.set<std::vector<VariableName>>("elements") = _configuration->element_variables;
    params.applySpecificParameters(parameters(), {"block", "temperature", "pressure"});
    _problem->addUserObject("ThermochimicaData", objectName(uniqueActionName().name()), params);
  }
#endif
}

void
ChemicalCompositionAction::readCSV()
{
  const auto & filename = getParam<FileName>("initial_composition_file");
  std::ifstream file(filename.c_str());
  if (!file.good())
    paramError("initial_composition_file", "Error opening file '", filename, "'.");

  std::string line;
  std::vector<std::string> items;
  std::getline(file, line);
  while (std::getline(file, line))
  {
    items.clear();
    MooseUtils::tokenize(line, items, 1, ",");
    if (items.empty())
      continue;
    if (items.size() != 2)
      paramError("initial_composition_file", "Unexpected CSV line: ", line);
    if (std::find(_configuration->elements.begin(), _configuration->elements.end(), items[0]) ==
        _configuration->elements.end())
      paramError("initial_composition_file", "Element '", items[0], "' is not configured.");
    _initial_conditions[items[0]] = MooseUtils::convert<Real>(items[1]);
  }
}
