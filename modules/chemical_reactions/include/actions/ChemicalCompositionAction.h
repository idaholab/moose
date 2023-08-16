//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AddFunctionAction.h"

/**
 * The ChemicalCompositionAction sets up user objects, aux kernels, and aux variables
 * for a thermochemistry calculation using Thermochimica.
 */
class ChemicalCompositionAction : public Action
{
public:
  static InputParameters validParams();
  ChemicalCompositionAction(const InputParameters & params);

  const std::vector<unsigned int> & elementIDs() const { return _element_ids; }

  const std::vector<std::string> & phases() const { return _phases; }
  const std::vector<std::string> & elementPotentials() const
  {
    return _tokenized_element_potentials;
  }

  const std::vector<std::pair<std::string, std::string>> & speciesPhasePairs() const
  {
    return _tokenized_species;
  }

  const std::vector<std::pair<std::string, std::string>> & vaporPhasePairs() const
  {
    return _tokenized_vapor_species;
  }

  const std::vector<std::pair<std::string, std::string>> & phaseElementPairs() const
  {
    return _tokenized_phase_elements;
  }

  const MooseEnum & outputSpeciesUnit() const { return _output_mass_unit; }

  const MooseEnum & reinitializationType() const { return _reinit; }

  virtual void act();

protected:
  void readCSV();

  /// Element names
  std::vector<std::string> _elements;

  /// Initial conditions for each element: [element name] => initial condition value
  std::map<std::string, Real> _initial_conditions;

  /// Temperature unit
  std::string _tunit;

  /// Pressure unit
  std::string _punit;

  /// Mass/amount unit
  std::string _munit;

  /// List of phases tracked by Thermochimica
  std::vector<std::string> _phases;

  /// List of species tracked by Thermochimica
  std::vector<std::string> _species;

  /// Mass unit for output species
  MooseEnum _output_mass_unit;

  /// List of element chemical potentials to be extracted from Thermochimica
  std::vector<std::string> _element_potentials;

  /// List of gas phase species to extract vapor pressures from Thermochimica
  std::vector<std::string> _vapor_pressures;

  /// List of elements in specific phases to extract the molar amount of the element in that phase
  std::vector<std::string> _element_phases;

  /// Flag for whether Thermochimica should use the reinit feature or not
  MooseEnum _reinit;

  /// Name of the ThermochimicaNodalData UO to be set up
  std::string _uo_name;

  /// Atomic numbers of the selected elements
  std::vector<unsigned int> _element_ids;

  /// Keep track of database
  static bool _database_parsed;
  static std::string _database_file;

  /// Tokenized versions of the output variables to avoid redoing tokenization
  std::vector<std::pair<std::string, std::string>> _tokenized_species;
  std::vector<std::string> _tokenized_element_potentials;
  std::vector<std::pair<std::string, std::string>> _tokenized_vapor_species;
  std::vector<std::pair<std::string, std::string>> _tokenized_phase_elements;
};
