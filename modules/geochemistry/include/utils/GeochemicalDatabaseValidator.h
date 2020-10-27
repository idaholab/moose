//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "nlohmann/json.h"
#include "MooseTypes.h"

/**
 * Class for validating MOOSE geochemical database
 */
class GeochemicalDatabaseValidator
{
public:
  GeochemicalDatabaseValidator(const FileName filename, const nlohmann::json & db);

  /**
   * Validate the thermodynamic database
   */
  void validate();

protected:
  /**
   * Check Json::Value can be converted to a Real
   * @param array array of values
   * @return true/false if the value can/cannot be converted to a Real
   */
  bool isValueReal(const nlohmann::json & value) const;

  /**
   * Check Json::Value array is comprised of Reals
   * @param array array of values
   * @param field database field name
   */
  void checkArrayValues(const nlohmann::json & array, const std::string field) const;

  /**
   * Check array array values can be converted to Real
   * @param type species type (basis, secondary, etc)
   * @param species species name
   * @param field name of array values to check
   */
  void checkArrayValues(const std::string type,
                        const std::string species,
                        const std::string field) const;

  /**
   * Check Json::Value array is the correct size
   * @param array array of values
   * @param field database field name
   */
  void checkArraySize(const nlohmann::json & array, const std::string field) const;

  /**
   * Check array is the correct size
   * @param type species type (basis, secondary, etc)
   * @param species species name
   * @param field name of array values to check
   */
  void
  checkArraySize(const std::string type, const std::string species, const std::string field) const;

  /**
   * Check fields are present in the Header section
   * @param field name of field to check
   */
  void checkHeaderField(const std::string field) const;

  /**
   * Check arrays in field can be converted to Reals
   * @param field name of field to check
   */
  void checkHeaderArray(const std::string field) const;

  /**
   * Check elements in the database
   * Each elemenent should have a real number as their molecular weights
   * @param element element name
   */
  void checkElements(const std::string element) const;

  /**
   * Check basis species in the database
   * Each basis species should have a real number as their molecular weights, charge, radius
   * and weight for each element
   * @param species basis species name
   */
  void checkBasisSpecies(const std::string species) const;

  /**
   * Check secondary species in the database
   * Each secondary species should have a real number as their molecular weights, charge, radius,
   * equilibrium constants and weight for each basis species
   * @param species secondary species name
   */
  void checkSecondarySpecies(const std::string species) const;

  /**
   * Check mineral species in the database
   * Each mineral species should have a real number as their molecular weights, volume,
   * equilibrium constants and weight for each basis species
   * @param species mineral species name
   */
  void checkMineralSpecies(const std::string species) const;

  /**
   * Check sorbing mineral species in the database
   * Each sorbing mineral species should have a real number as their surface area,
   * equilibrium constants and weight for each sorbing site
   * @param species sorbing mineral species name
   */
  void checkSorbingMineralSpecies(const std::string species) const;

  /**
   * Check gas species in the database
   * Each gas species should have a real number as their molecular weights,
   * equilibrium constants, weight for each basis species, and fugacity values
   * @param species gas species name
   */
  void checkGasSpecies(const std::string species) const;

  /**
   * Check redox couple species in the database
   * Each redox couple species should have a real number as their molecular weights, charge, radius,
   * equilibrium constants and weight for each basis species
   * @param species redox couple species name
   */
  void checkRedoxSpecies(const std::string species) const;

  /**
   * Check oxide species in the database
   * Each oxide species should have a real number as their molecular weights
   * and weight for each basis species
   * @param species oxide species name
   */
  void checkOxideSpecies(const std::string species) const;

  /**
   * Check surface species in the database
   * Each surface species should have a real number as their molecular weights, charge,
   * and equilibrium constants
   * @param species surface species name
   */
  void checkSurfaceSpecies(const std::string species) const;

  /**
   * Check given species field value can be converted to a Real
   * @param type species type (basis, secondary, etc)
   * @param species species name
   * @param field value to check
   */
  void checkSpeciesValue(const std::string type,
                         const std::string species,
                         const std::string field) const;

  /**
   * Check given species stoichiometric weigth values can be converted to a Real
   * @param type species type (basis, secondary, etc)
   * @param species species name
   * @param field name of values to check
   */
  void checkSpeciesWeightValue(const std::string type,
                               const std::string species,
                               const std::string field) const;

  /// Database filename
  const FileName _filename;
  /// JSON database
  const nlohmann::json & _root;
  /// Number of temperature points
  unsigned int _temperature_size;
};
