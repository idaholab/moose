//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "json/json.h"
#include "MooseTypes.h"

/**
 * Data structure for basis (primary) species.
 * Members are:
 * * species name
 * * elemental composition (elements and weights)
 * * ionic radius (Angstrom)
 * * charge
 * * molecular weight (g)
 */
struct GeochemistryBasisSpecies
{
  GeochemistryBasisSpecies(){};

  std::string name;
  std::map<std::string, Real> elements;
  Real radius;
  Real charge;
  Real molecular_weight;
};

/**
 * Data structure for secondary equilibrium species.
 * Members are:
 * * Species name
 * * Basis species and stoichiometric coefficients present
 * * Equilibrium constant at given temperatures
 * * ionic radius (Angstrom)
 * * charge
 * * molecular weight (g)
 */
struct GeochemistryEquilibriumSpecies
{
  GeochemistryEquilibriumSpecies(){};

  std::string name;
  std::map<std::string, Real> basis_species;
  std::vector<Real> equilibrium_const;
  Real radius;
  Real charge;
  Real molecular_weight;
};

/**
 * Data structure for mineral species.
 * Members are:
 * * Species name
 * * Molecular volume (units?)
 * * Basis species and stoichiometric coefficients present
 * * Equilibrium constant at given temperatures ()
 * * molecular weight (g)
 */
struct GeochemistryMineralSpecies
{
  GeochemistryMineralSpecies(){};

  std::string name;
  Real molecular_volume;
  std::map<std::string, Real> basis_species;
  std::vector<Real> equilibrium_const;
  Real molecular_weight;
};

/**
 * Data structure for mineral species.
 * Members are:
 * * Species name
 * * Basis species and stoichiometric coefficients present
 * * Equilibrium constant at given temperatures ()
 * * molecular weight (g)
 * * Spycher-Reed fugacity coefficients
 * * Pcrit, Tcrit and omega - Tsonopoulos fugacity coefficients
 */
struct GeochemistryGasSpecies
{
  GeochemistryGasSpecies(){};

  std::string name;
  std::map<std::string, Real> basis_species;
  std::vector<Real> equilibrium_const;
  Real molecular_weight;
  std::vector<Real> chi;
  Real Pcrit;
  Real Tcrit;
  Real omega;
};

/**
 * Data structure for redox species.
 * Members are:
 * * Species name
 * * Basis species and stoichiometric coefficients present
 * * Equilibrium constant at given temperatures
 * * ionic radius (Angstrom)
 * * charge
 * * molecular weight (g)
 */
struct GeochemistryRedoxSpecies
{
  GeochemistryRedoxSpecies(){};

  std::string name;
  std::map<std::string, Real> basis_species;
  std::vector<Real> equilibrium_const;
  Real radius;
  Real charge;
  Real molecular_weight;
};

/**
 * Data structure for oxide species.
 * Members are:
 * * Species name
 * * Basis species and stoichiometric coefficients present
 * * molecular weight (g)
 */
struct GeochemistryOxideSpecies
{
  GeochemistryOxideSpecies(){};

  std::string name;
  std::map<std::string, Real> basis_species;
  Real molecular_weight;
};

/**
 * Data structure for Debye-Huckel activity coefficients.
 * Members are:
 * * adh: Debye-Huckel a parameter
 * * bdh: Debye-Huckel b parameter
 * * bdot: Debye-Huckel bdot parameter
 */
struct GeochemistryDebyeHuckel
{
  GeochemistryDebyeHuckel(){};

  std::vector<Real> adh;
  std::vector<Real> bdh;
  std::vector<Real> bdot;
};

/**
 * Data structure for elements.
 * Members are:
 * * name: Element name
 * * molecular_weight: Element molecular weight
 */
struct GeochemistryElements
{
  GeochemistryElements(){};

  std::string name;
  Real molecular_weight;
};

/**
 * Data structure for neutral species activity coefficients.
 * Members are:
 * * adh: Debye-Huckel a parameter
 * * bdh: Debye-Huckel b parameter
 * * bdot: Debye-Huckel bdot parameter
 */
struct GeochemistryNeutralSpeciesActivity
{
  GeochemistryNeutralSpeciesActivity(){};
  GeochemistryNeutralSpeciesActivity(std::vector<std::vector<Real>> coeffs)
    : a(coeffs[0]), b(coeffs[1]), c(coeffs[2]), d(coeffs[3]){};

  std::vector<Real> a;
  std::vector<Real> b;
  std::vector<Real> c;
  std::vector<Real> d;
};

/**
 * Class for reading geochemical reactions from a MOOSE geochemical database
 */
class GeochemicalDatabaseReader
{
public:
  GeochemicalDatabaseReader(const std::string filename);

  /**
   * Parse the thermodynamic database
   */
  void read(FileName filename);

  /**
   * Get the activity model type
   * @retrun activity model
   */
  std::string getActivityModel() const;

  /**
   * Get the fugacity model type
   * @retrun fugacity model
   */
  std::string getFugacityModel() const;

  /**
   * Get the list of basis (primary) species read from database
   * @return list of primary species names
   */
  std::vector<std::string> getBasisSpeciesNames() const { return _bs_names; };

  /**
   * Get the list of secondary equilibrium species read from database
   * @return list of equilibrium species names
   */
  std::vector<std::string> getEquilibriumSpeciesNames() const { return _es_names; };

  /**
   * Get the list of secondary mineral species read from database
   * @return list of mineral species names
   */
  std::vector<std::string> getMineralSpeciesNames() const { return _ms_names; };

  /**
   * Get the temperature points that the equilibrium constant is defined at.
   * @return vector of temperature points (C)
   */
  std::vector<Real> getTemperatures();

  /**
   * Get the pressure points that the equilibrium constant is defined at.
   * @return vector of pressure points (C)
   */
  std::vector<Real> getPressures();

  /**
   * Get the Debye-Huckel activity coefficients
   * @return vectors of adh, bdh and bdot
   */
  GeochemistryDebyeHuckel getDebyeHuckel();

  /**
   * Get the basis (primary) species information
   * @param names list of basis species
   * @return basis species structure
   */
  std::map<std::string, GeochemistryBasisSpecies>
  getBasisSpecies(const std::vector<std::string> & names);

  /**
   * Get the secondary equilibrium species information
   * @param names list of equilibrium species
   * @return secondary species structure
   */
  std::map<std::string, GeochemistryEquilibriumSpecies>
  getEquilibriumSpecies(const std::vector<std::string> & names);

  /**
   * Get the mineral species information
   * @param names list of mineral species
   * @return mineral species structure
   */
  std::map<std::string, GeochemistryMineralSpecies>
  getMineralSpecies(const std::vector<std::string> & names);

  /**
   * Get all the elements
   * @return elements species structure
   */
  std::map<std::string, GeochemistryElements> getElements();

  /**
   * Get the gas species information
   * @param names list of gs species
   * @return gas species structure
   */
  std::map<std::string, GeochemistryGasSpecies>
  getGasSpecies(const std::vector<std::string> & names);

  /**
   * Get the redox species (couples) information
   * @param names list of gs species
   * @return redox species structure
   */
  std::map<std::string, GeochemistryRedoxSpecies>
  getRedoxSpecies(const std::vector<std::string> & names);

  /**
   * Get the oxide species information
   * @param names list of gs species
   * @return oxide species structure
   */
  std::map<std::string, GeochemistryOxideSpecies>
  getOxideSpecies(const std::vector<std::string> & names);

  /**
   * Get the neutral species activity coefficients
   * @return neutral species activity coefficients
   */
  std::map<std::string, GeochemistryNeutralSpeciesActivity> getNeutralSpeciesActivity();

  /**
   * Generates a formatted vector of strings representing all aqueous equilibrium
   * reactions
   * @param names list of equilibrium species
   * return formatted equilibrium reactions
   */
  std::vector<std::string> equilibriumReactions(const std::vector<std::string> & names) const;

  /**
   * Generates a formatted vector of strings representing all mineral reactions
   * @param names list of mineral species
   * @preturn formatted mineral reactions
   */
  std::vector<std::string> mineralReactions(const std::vector<std::string> & names) const;

  /**
   * Generates a formatted vector of strings representing all gas reactions
   * @param names list of gas species
   * @preturn formatted gas reactions
   */
  std::vector<std::string> gasReactions(const std::vector<std::string> & names) const;

  /**
   * Generates a formatted vector of strings representing all redox reactions
   * @param names list of redox species
   * @preturn formatted redox reactions
   */
  std::vector<std::string> redoxReactions(const std::vector<std::string> & names) const;

  /**
   * Generates a formatted vector of strings representing all oxide reactions
   * @param names list of oxide species
   * @preturn formatted oxide reactions
   */
  std::vector<std::string> oxideReactions(const std::vector<std::string> & names) const;

protected:
  /**
   * Generates a formatted vector of strings representing all reactions
   * @param names list of reaction species
   * @param basis species list of basis species for each reaction species
   * @preturn formatted reaction equations
   */
  std::vector<std::string>
  printReactions(std::vector<std::string> names,
                 std::vector<std::map<std::string, Real>> basis_species) const;

  /// Database filename
  const FileName _filename;
  /// JSON data
  moosecontrib::Json::Value _root;
  /// List of basis (primary) species names to read from database
  std::vector<std::string> _bs_names;
  /// List of secondary equilibrium species to read from database
  std::vector<std::string> _es_names;
  /// List of secondary mineral species to read from database
  std::vector<std::string> _ms_names;
  /// Temperature points in database
  std::vector<Real> _temperature_points;
  /// Pressure points in database
  std::vector<Real> _pressure_points;
  /// Elements and their molecular weight read from the database
  std::map<std::string, GeochemistryElements> _elements;
  /// Basis species data read from the database
  std::map<std::string, GeochemistryBasisSpecies> _basis_species;
  /// Secondary equilibrium species data read from the database
  std::map<std::string, GeochemistryEquilibriumSpecies> _equilibrium_species;
  /// Mineral species data read from the database
  std::map<std::string, GeochemistryMineralSpecies> _mineral_species;
  /// Gas species data read from the database
  std::map<std::string, GeochemistryGasSpecies> _gas_species;
  /// Redox species (couples) data read from the database
  std::map<std::string, GeochemistryRedoxSpecies> _redox_species;
  /// Oxide species data read from the database
  std::map<std::string, GeochemistryOxideSpecies> _oxide_species;
  /// Debye-Huckel activity coefficients
  GeochemistryDebyeHuckel _debye_huckel;
  /// Neutral species activity coefficients
  std::map<std::string, GeochemistryNeutralSpeciesActivity> _neutral_species_activity;
};
