//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "ThermochimicaConfiguration.h"

/**
 * The ChemicalCompositionAction sets up user objects, aux kernels, and aux variables
 * for a thermochemistry calculation using Thermochimica.
 */
class ChemicalCompositionAction : public Action
{
public:
  static InputParameters validParams();
  ChemicalCompositionAction(const InputParameters & params);

  virtual void act() override;

protected:
  /// Build and validate the immutable configuration after all input Actions have been parsed.
  void initializeConfiguration();

  void readCSV();

#ifdef THERMOCHIMICA_ENABLED
  /** Translate the legacy flat output parameters into typed output descriptors. */
  void buildLegacyOutputDescriptors(const std::vector<std::string> & database_phases,
                                    const std::vector<std::vector<std::string>> & database_species);

  /** Validate and append an output descriptor produced by any input syntax. */
  void addOutputDescriptor(ThermochimicaConfiguration::OutputDescriptor output,
                           const std::string & origin);
#endif

  /// Initial conditions for each element: [element name] => initial condition value
  std::map<std::string, Real> _initial_conditions;

  /// Validated immutable configuration consumed by the runtime executor
  std::shared_ptr<ThermochimicaConfiguration> _configuration;

  /// Input parameter or block that generated each configured variable
  std::map<VariableName, std::string> _variable_origins;
};
