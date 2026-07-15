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

struct ThermochimicaPhaseAmountRequest
{
  VariableName variable;
  std::string phase;
};

struct ThermochimicaSpeciesAmountRequest
{
  VariableName variable;
  std::string phase;
  std::string species;
  ThermochimicaConfiguration::SpeciesUnit unit;
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

struct ThermochimicaElementInPhaseRequest
{
  VariableName variable;
  std::string phase;
  std::string element;
};

using ThermochimicaOutputRequest = std::variant<ThermochimicaPhaseAmountRequest,
                                                ThermochimicaSpeciesAmountRequest,
                                                ThermochimicaElementPotentialRequest,
                                                ThermochimicaVaporPressureRequest,
                                                ThermochimicaElementInPhaseRequest>;

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

class ThermochimicaPhaseAmountOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaPhaseAmountOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};

class ThermochimicaSpeciesAmountOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaSpeciesAmountOutputAction(const InputParameters & parameters);

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

class ThermochimicaElementInPhaseOutputAction : public ThermochimicaOutputAction
{
public:
  static InputParameters validParams();
  ThermochimicaElementInPhaseOutputAction(const InputParameters & parameters);

  virtual ThermochimicaOutputRequest request() const override;
};
