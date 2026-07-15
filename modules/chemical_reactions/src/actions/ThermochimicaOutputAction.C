//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "ThermochimicaOutputAction.h"

registerMooseAction("ChemicalReactionsApp",
                    ThermochimicaPhaseAmountOutputAction,
                    "setup_chemical_composition");
registerMooseAction("ChemicalReactionsApp",
                    ThermochimicaSpeciesAmountOutputAction,
                    "setup_chemical_composition");
registerMooseAction("ChemicalReactionsApp",
                    ThermochimicaElementPotentialOutputAction,
                    "setup_chemical_composition");
registerMooseAction("ChemicalReactionsApp",
                    ThermochimicaVaporPressureOutputAction,
                    "setup_chemical_composition");
registerMooseAction("ChemicalReactionsApp",
                    ThermochimicaElementInPhaseOutputAction,
                    "setup_chemical_composition");

InputParameters
ThermochimicaOutputAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<VariableName>(
      "variable", "Override the output variable name, which otherwise uses the block name");
  return params;
}

ThermochimicaOutputAction::ThermochimicaOutputAction(const InputParameters & parameters)
  : Action(parameters),
    _variable(isParamValid("variable") ? getParam<VariableName>("variable") : name()),
    _origin(parameters.blockFullpath()),
    _parent_path(
        [&]()
        {
          const auto & path = parameters.blockFullpath();
          const auto separator = path.rfind("/Outputs/");
          if (separator == std::string::npos)
            mooseError("Typed Thermochimica output block has invalid path '", path, "'.");
          return path.substr(0, separator);
        }())
{
}

InputParameters
ThermochimicaPhaseAmountOutputAction::validParams()
{
  InputParameters params = ThermochimicaOutputAction::validParams();
  params.addClassDescription("Selects a phase amount from a Thermochimica equilibrium result.");
  params.addRequiredParam<std::string>("phase", "Phase whose equilibrium amount is output");
  return params;
}

ThermochimicaPhaseAmountOutputAction::ThermochimicaPhaseAmountOutputAction(
    const InputParameters & parameters)
  : ThermochimicaOutputAction(parameters)
{
}

ThermochimicaOutputRequest
ThermochimicaPhaseAmountOutputAction::request() const
{
  return ThermochimicaPhaseAmountRequest{_variable, getParam<std::string>("phase")};
}

InputParameters
ThermochimicaSpeciesAmountOutputAction::validParams()
{
  InputParameters params = ThermochimicaOutputAction::validParams();
  params.addClassDescription("Selects a species amount from a Thermochimica equilibrium result.");
  params.addRequiredParam<std::string>("phase", "Phase containing the species");
  params.addRequiredParam<std::string>("species", "Species whose equilibrium amount is output");
  params.addParam<MooseEnum>(
      "unit", MooseEnum("moles mole_fraction", "moles"), "Unit used for this species output");
  return params;
}

ThermochimicaSpeciesAmountOutputAction::ThermochimicaSpeciesAmountOutputAction(
    const InputParameters & parameters)
  : ThermochimicaOutputAction(parameters)
{
}

ThermochimicaOutputRequest
ThermochimicaSpeciesAmountOutputAction::request() const
{
  const auto unit = getParam<MooseEnum>("unit") == "moles"
                        ? ThermochimicaConfiguration::SpeciesUnit::MOLES
                        : ThermochimicaConfiguration::SpeciesUnit::MOLE_FRACTION;
  return ThermochimicaSpeciesAmountRequest{
      _variable, getParam<std::string>("phase"), getParam<std::string>("species"), unit};
}

InputParameters
ThermochimicaElementPotentialOutputAction::validParams()
{
  InputParameters params = ThermochimicaOutputAction::validParams();
  params.addClassDescription(
      "Selects an element chemical potential from a Thermochimica equilibrium result.");
  params.addRequiredParam<std::string>("element", "Element whose chemical potential is output");
  return params;
}

ThermochimicaElementPotentialOutputAction::ThermochimicaElementPotentialOutputAction(
    const InputParameters & parameters)
  : ThermochimicaOutputAction(parameters)
{
}

ThermochimicaOutputRequest
ThermochimicaElementPotentialOutputAction::request() const
{
  return ThermochimicaElementPotentialRequest{_variable, getParam<std::string>("element")};
}

InputParameters
ThermochimicaVaporPressureOutputAction::validParams()
{
  InputParameters params = ThermochimicaOutputAction::validParams();
  params.addClassDescription(
      "Selects a species vapor pressure from a Thermochimica equilibrium result.");
  params.addRequiredParam<std::string>("phase", "Gas phase containing the species");
  params.addRequiredParam<std::string>("species", "Species whose vapor pressure is output");
  return params;
}

ThermochimicaVaporPressureOutputAction::ThermochimicaVaporPressureOutputAction(
    const InputParameters & parameters)
  : ThermochimicaOutputAction(parameters)
{
}

ThermochimicaOutputRequest
ThermochimicaVaporPressureOutputAction::request() const
{
  return ThermochimicaVaporPressureRequest{
      _variable, getParam<std::string>("phase"), getParam<std::string>("species")};
}

InputParameters
ThermochimicaElementInPhaseOutputAction::validParams()
{
  InputParameters params = ThermochimicaOutputAction::validParams();
  params.addClassDescription(
      "Selects an element amount in a phase from a Thermochimica equilibrium result.");
  params.addRequiredParam<std::string>("phase", "Phase containing the element");
  params.addRequiredParam<std::string>("element", "Element whose amount in the phase is output");
  return params;
}

ThermochimicaElementInPhaseOutputAction::ThermochimicaElementInPhaseOutputAction(
    const InputParameters & parameters)
  : ThermochimicaOutputAction(parameters)
{
}

ThermochimicaOutputRequest
ThermochimicaElementInPhaseOutputAction::request() const
{
  return ThermochimicaElementInPhaseRequest{
      _variable, getParam<std::string>("phase"), getParam<std::string>("element")};
}
