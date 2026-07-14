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

/** Immutable setup data shared by the ChemicalComposition action and its runtime executor. */
struct ThermochimicaConfiguration
{
  enum class EvaluationLocation : unsigned char
  {
    NODAL,
    ELEMENTAL
  };

  enum class WarmStart : unsigned char
  {
    PREVIOUS_SOLVE,
    PREVIOUS_TIMESTEP,
    NONE
  };

  enum class SpeciesUnit : unsigned char
  {
    MOLES,
    MOLE_FRACTION
  };

  struct SpeciesOutput
  {
    std::string phase;
    std::string species;
    int phase_index = -1;
    int species_index = -1;
    bool is_mqm = false;
  };

  struct ElementPhaseOutput
  {
    std::string phase;
    std::string element;
    int phase_index = -1;
    int element_index = -1;
  };

  FileName database;
  std::string temperature_unit;
  std::string pressure_unit;
  std::string composition_unit;
  std::string temperature;
  std::string pressure;
  EvaluationLocation location = EvaluationLocation::NODAL;
  WarmStart warm_start = WarmStart::PREVIOUS_SOLVE;
  SpeciesUnit species_unit = SpeciesUnit::MOLES;
  unsigned int batch_size = 32;
  bool report_performance = false;

  std::vector<std::string> elements;
  std::vector<unsigned int> element_ids;
  std::vector<std::string> phases;
  std::vector<int> phase_indices;
  std::vector<SpeciesOutput> species;
  std::vector<std::string> element_potentials;
  std::vector<int> element_potential_indices;
  std::vector<SpeciesOutput> vapor_species;
  std::vector<ElementPhaseOutput> phase_elements;

  std::vector<VariableName> element_variables;
  std::vector<VariableName> output_variables;

  std::size_t inputWidth() const { return 2 + elements.size(); }
  std::size_t outputWidth() const { return output_variables.size(); }
};

using ThermochimicaConfigurationPtr = std::shared_ptr<const ThermochimicaConfiguration>;
