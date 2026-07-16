//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include <memory>
#include <variant>

/** Immutable setup data shared by the ChemicalComposition action and its runtime executor. */
struct ThermochimicaConfiguration
{
  enum class EvaluationLocation
  {
    NODAL,
    ELEMENTAL
  };

  enum class WarmStart
  {
    PREVIOUS_SOLVE,
    PREVIOUS_TIMESTEP,
    NONE
  };

  enum class AmountUnit
  {
    MOLES,
    MOLE_FRACTION
  };

  enum class DistributionUnit
  {
    MOLES,
    FRACTION
  };

  struct PhaseOutput
  {
    VariableName variable;
    std::string phase;
    int phase_index = -1;
    AmountUnit unit = AmountUnit::MOLES;
  };

  struct SpeciesOutput
  {
    VariableName variable;
    std::string phase;
    std::string species;
    int phase_index = -1;
    int species_index = -1;
    bool is_mqm = false;
    AmountUnit unit = AmountUnit::MOLES;
  };

  struct ElementPotentialOutput
  {
    VariableName variable;
    std::string element;
    int element_index = -1;
  };

  struct VaporPressureOutput
  {
    VariableName variable;
    std::string phase;
    std::string species;
    int phase_index = -1;
    int species_index = -1;
  };

  struct ElementDistributionOutput
  {
    VariableName variable;
    std::string phase;
    std::string element;
    int phase_index = -1;
    int element_index = -1;
    DistributionUnit unit = DistributionUnit::MOLES;
  };

  using OutputDescriptor = std::variant<PhaseOutput,
                                        SpeciesOutput,
                                        ElementPotentialOutput,
                                        VaporPressureOutput,
                                        ElementDistributionOutput>;

  FileName database;
  std::string temperature_unit;
  std::string pressure_unit;
  std::string composition_unit;
  std::string temperature;
  std::string pressure;
  EvaluationLocation location = EvaluationLocation::NODAL;
  WarmStart warm_start = WarmStart::PREVIOUS_SOLVE;
  unsigned int batch_size = 32;
  bool report_performance = false;

  std::vector<std::string> elements;
  std::vector<unsigned int> element_ids;
  std::vector<std::string> phase_names;
  std::vector<int> phase_indices;
  bool needs_phase_total = false;
  std::vector<OutputDescriptor> outputs;

  std::vector<VariableName> element_variables;

  std::size_t inputWidth() const { return 2 + elements.size(); }
  std::size_t outputWidth() const { return outputs.size(); }
};

using ThermochimicaConfigurationPtr = std::shared_ptr<const ThermochimicaConfiguration>;
