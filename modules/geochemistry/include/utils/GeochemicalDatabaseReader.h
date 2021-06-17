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
 * * specific surface area (m2/g) (only nonzero for sorbing minerals)
 * * basis species (sorbing sites) and their site density (only populated for sorbing minerals)
 */
struct GeochemistryMineralSpecies
{
  GeochemistryMineralSpecies(){};

  std::string name;
  Real molecular_volume;
  std::map<std::string, Real> basis_species;
  std::vector<Real> equilibrium_const;
  Real molecular_weight;
  Real surface_area;
  std::map<std::string, Real> sorption_sites;
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
 * Data structure for sorbing surface species.
 * * Species name
 * * Basis species and stoichiometric coefficients present
 * * log10(Equilibrium constant) and dlog10K/dT
 * * charge
 * * molecular weight (g)
 */
struct GeochemistrySurfaceSpecies
{
  GeochemistrySurfaceSpecies(){};

  std::string name;
  std::map<std::string, Real> basis_species;
  Real charge;
  Real molecular_weight;
  Real log10K;
  Real dlog10KdT;
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
 * Members are a, b, c and d, which are temperature dependent coefficients in the Debye-Huckel
 * activity model.
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
  /**
   * Parse the file
   * @param filename Moose geochemical database file
   * @param reexpress_free_electron If true, and if the free electron in the database file has an
   * equilibrium reaction expressed in terms of O2(g), and O2(g) exists as a gas in the database
   * file, and O2(g)'s equilibrium reaction is O2(g)=O2(eq), and O2(aq) exists as a basis species in
   * the database file, then reexpress the free electron's equilibrium reaction in terms of O2(aq)
   * @param use_piecewise_interpolation If true then set the "logk model" to "piecewise-linear"
   * regardless of the value found in the filename.  This is designed to make testing easy (because
   * logK and Debye-Huckel parameters will be exactly as set in the filename instead of from a 4-th
   * order least-squares fit) but should rarely be used for real geochemical simulations
   */
  GeochemicalDatabaseReader(const FileName filename,
                            const bool reexpress_free_electron = true,
                            const bool use_piecewise_interpolation = false,
                            const bool remove_all_extrapolated_secondary_species = false);

  /**
   * Parse the thermodynamic database
   * @param filename Name of thermodynamic database file
   */
  void read(const FileName filename);

  /**
   * Validate the thermodynamic database
   * @param filename Name of thermodynamic database file
   * @param db JSON database read from filename
   */
  void validate(const FileName filename, const nlohmann::json & db);

  /**
   * Sometimes the free electron's equilibrium reaction is defined in terms of O2(g) which is not a
   * basis species.  If this is the case, re-express it in terms of O2(aq), if O2(g) is a gas and
   * O2(aq) is a basis species.
   */
  void reexpressFreeElectron();

  /**
   * Get the activity model type
   * @return activity model
   */
  std::string getActivityModel() const;

  /**
   * Get the fugacity model type
   * @return fugacity model
   */
  std::string getFugacityModel() const;

  /**
   * Get the equilibrium constant model type
   * @return equilibrium constant model
   */
  std::string getLogKModel() const;

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
  const std::vector<Real> & getTemperatures() const;

  /**
   * Get the pressure points that the equilibrium constant is defined at.
   * @return vector of pressure points (C)
   */
  std::vector<Real> getPressures();

  /**
   * Get the Debye-Huckel activity coefficients
   * @return vectors of adh, bdh and bdot
   */
  const GeochemistryDebyeHuckel & getDebyeHuckel() const;

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
   * Get the surface sorbing species information
   * @param names list of surface sorbing species
   * @return surface sorbing species structure
   */
  std::map<std::string, GeochemistrySurfaceSpecies>
  getSurfaceSpecies(const std::vector<std::string> & names);

  /**
   * Get the neutral species activity coefficients
   * @return neutral species activity coefficients
   */
  const std::map<std::string, GeochemistryNeutralSpeciesActivity> &
  getNeutralSpeciesActivity() const;

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

  /**
   * String representation of JSON species object contents
   * @param name name of species
   * @return styled string of species information
   */
  std::string getSpeciesData(const std::string name) const;

  /**
   * Filename of database
   * @return filename
   */
  const FileName & filename() const;

  /// Returns true if name is a "secondary species" or "free electron" in the database
  bool isSecondarySpecies(const std::string & name) const;

  /**
   * Checks if species is of given type
   * @param name species name
   * @return true iff species is of given type
   */
  bool isBasisSpecies(const std::string & name) const;
  bool isRedoxSpecies(const std::string & name) const;
  bool isGasSpecies(const std::string & name) const;
  bool isMineralSpecies(const std::string & name) const;
  bool isOxideSpecies(const std::string & name) const;
  bool isSurfaceSpecies(const std::string & name) const;

  /// returns True iff name is the name of a sorbing mineral
  bool isSorbingMineral(const std::string & name) const;

  /// Returns a list of all the names of the "mineral species" in the database
  std::vector<std::string> mineralSpeciesNames() const;

  /// Returns a list of all the names of the "secondary species" and "free electron" in the database
  std::vector<std::string> secondarySpeciesNames() const;

  /// Returns a list of all the names of the "redox couples" in the database
  std::vector<std::string> redoxCoupleNames() const;

  /// Returns a list of all the names of the "surface species" in the database
  std::vector<std::string> surfaceSpeciesNames() const;

protected:
  /**
   * After parsing the database file, remove any secondary species that have extrapolated
   * equilibrium constants.  This is called in the constructor if the
   * remove_all_extrapolated_secondary_species flag is true
   */
  void removeExtrapolatedSecondarySpecies();

  /**
   * Copy the temperature points (if any) found in the database into _temperature_points.  This
   * method is called in the constructor
   */
  void setTemperatures();

  /**
   * Copy the Debye-Huckel parameters (if any) found in the database into _debye_huckel.  This
   * method is called in the constructor
   */
  void setDebyeHuckel();

  /**
   * Copy the Debye-Huckel parameters for computing neutral species activity (if any) found in the
   * databasebase into _neutral_species_activity.  This method is cdalled in the constructor
   */
  void setNeutralSpeciesActivity();

  /// Database filename
  const FileName _filename;
  /// JSON data
  nlohmann::json _root;
  /// List of basis (primary) species names read from database
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
  /// Secondary equilibrium species and free electron data read from the database
  std::map<std::string, GeochemistryEquilibriumSpecies> _equilibrium_species;
  /// Mineral species data read from the database
  std::map<std::string, GeochemistryMineralSpecies> _mineral_species;
  /// Gas species data read from the database
  std::map<std::string, GeochemistryGasSpecies> _gas_species;
  /// Redox species (couples) data read from the database
  std::map<std::string, GeochemistryRedoxSpecies> _redox_species;
  /// Oxide species data read from the database
  std::map<std::string, GeochemistryOxideSpecies> _oxide_species;
  /// Surface sorbing species data read from the database
  std::map<std::string, GeochemistrySurfaceSpecies> _surface_species;
  /// Debye-Huckel activity coefficients
  GeochemistryDebyeHuckel _debye_huckel;
  /// Neutral species activity coefficients
  std::map<std::string, GeochemistryNeutralSpeciesActivity> _neutral_species_activity;
  // Helper for converting json node to Real from string
  static Real getReal(const nlohmann::json & node);

private:
  /**
   * Generates a formatted vector of strings representing all reactions
   * @param names list of reaction species
   * @param basis species list of basis species for each reaction species (this vector must be of
   * same size as names)
   * @return formatted reaction equations
   */
  std::vector<std::string>
  printReactions(const std::vector<std::string> & names,
                 const std::vector<std::map<std::string, Real>> & basis_species) const;
};
