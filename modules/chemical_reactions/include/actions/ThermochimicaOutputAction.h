//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "Action.h"
#include "ThermochimicaConfiguration.h"

#include <variant>

struct ThermochimicaPhaseRequest
{
  VariableName variable;
  std::string phase;
  ThermochimicaConfiguration::AmountUnit unit;
};

struct ThermochimicaSpeciesRequest
{
  VariableName variable;
  std::string phase;
  std::string species;
  ThermochimicaConfiguration::AmountUnit unit;
};

struct ThermochimicaElementPotentialRequest
{
  VariableName variable;
  std::string element;
};

struct ThermochimicaVaporPressureRequest
{
  VariableName variable;
  std::string phase;
  std::string species;
};

struct ThermochimicaElementDistributionRequest
{
  VariableName variable;
  std::string phase;
  std::string element;
  ThermochimicaConfiguration::DistributionUnit unit;
};

struct ThermochimicaChemicalPotentialRequest
{
  VariableName variable;
  std::string phase;
  std::string component;
  std::string component_parameter;
  ThermochimicaConfiguration::ChemicalPotentialKind kind;
};

struct ThermochimicaPhaseGibbsEnergyRequest
{
  VariableName variable;
  std::string phase;
  ThermochimicaConfiguration::GibbsEnergyUnit unit;
};

struct ThermochimicaPhaseDrivingForceRequest
{
  VariableName variable;
  std::string phase;
};

struct ThermochimicaSystemGibbsEnergyRequest
{
  VariableName variable;
};

struct ThermochimicaSystemPropertyRequest
{
  VariableName variable;
  ThermochimicaConfiguration::SystemPropertyKind property;
};

struct ThermochimicaConstituentFractionRequest
{
  VariableName variable;
  std::string phase;
  unsigned int sublattice;
  std::string constituent;
};

using ThermochimicaOutputRequest = std::variant<ThermochimicaPhaseRequest,
                                                ThermochimicaSpeciesRequest,
                                                ThermochimicaElementPotentialRequest,
                                                ThermochimicaVaporPressureRequest,
                                                ThermochimicaElementDistributionRequest,
                                                ThermochimicaChemicalPotentialRequest,
                                                ThermochimicaPhaseGibbsEnergyRequest,
                                                ThermochimicaPhaseDrivingForceRequest,
                                                ThermochimicaSystemGibbsEnergyRequest,
                                                ThermochimicaSystemPropertyRequest,
                                                ThermochimicaConstituentFractionRequest>;

/** Base class for typed Thermochimica output selection Actions. */
class ThermochimicaOutputAction : public Action
{
public:
  static InputParameters validParams();
  ThermochimicaOutputAction(const InputParameters & parameters);

  virtual void act() override {}

  virtual ThermochimicaOutputRequest request() const = 0;

  const std::string & parentPath() const { return _parent_path; }
  const std::string & origin() const { return _origin; }

protected:
  const VariableName _variable;

private:
  const std::string _origin;
  const std::string _parent_path;
};

class ThermochimicaPhaseOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaPhaseOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaSpeciesOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaSpeciesOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaElementPotentialOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaElementPotentialOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaVaporPressureOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaVaporPressureOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaElementDistributionOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaElementDistributionOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaChemicalPotentialOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaChemicalPotentialOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaPhaseGibbsEnergyOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaPhaseGibbsEnergyOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaPhaseDrivingForceOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaPhaseDrivingForceOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaSystemGibbsEnergyOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaSystemGibbsEnergyOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaSystemPropertyOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaSystemPropertyOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaConstituentFractionOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaConstituentFractionOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};
