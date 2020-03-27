//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MinimalGeochemicalSystem.h"

MinimalGeochemicalSystem::MinimalGeochemicalSystem(
    const GeochemicalDatabaseReader & db,
    const std::vector<std::string> & basis_species,
    const std::vector<std::string> & minerals,
    const std::vector<std::string> & gases,
    const std::vector<std::string> & kinetic_minerals,
    const std::vector<std::string> & kinetic_redox,
    const std::vector<std::string> & kinetic_surface_species)
  : _db(db),
    _basis_index(),
    _basis_info(),
    _mineral_index(),
    _mineral_info(),
    _gas_index(),
    _gas_info(),
    _kinetic_mineral_index(),
    _kinetic_mineral_info(),
    _kinetic_redox_index(),
    _kinetic_redox_info(),
    _kinetic_surface_index(),
    _kinetic_surface_info(),
    _secondary_index(),
    _secondary_info(),
    _model()
{
  // Use the constructor info to build the "index" and "info" structures
  buildBasis(basis_species);
  buildMinerals(minerals);
  buildGases(gases);
  buildKineticMinerals(kinetic_minerals);
  buildKineticRedox(kinetic_redox);
  buildKineticSurface(kinetic_surface_species);

  // Pull out all secondary equilibrium species: these are all relevant "redox couples" and
  // "secondary species" and "surface species"
  buildSecondarySpecies();

  // Check that everything can be expressed in terms of the basis, possibly via redox and secondary
  // species
  checkMinerals(_mineral_info);
  checkGases();
  checkMinerals(_kinetic_mineral_info);
  checkKineticRedox();
  checkKineticSurfaceSpecies();

  // Populate _model
  createModel();
}

void
MinimalGeochemicalSystem::buildBasis(const std::vector<std::string> & basis_species)
{
  unsigned ind = 0;
  for (const auto & name : basis_species)
  {
    if (ind == 0 and name != "H2O")
      mooseError("First member of basis species list must be H2O");
    if (_basis_index.count(name) == 1)
      mooseError(name, " exists more than once in the basis species list");
    _basis_index.emplace(name, ind);
    ind += 1;
    if (_db.isBasisSpecies(name))
      _basis_info.push_back(_db.getBasisSpecies({name})[name]);
    else if (_db.isRedoxSpecies(name))
    {
      const GeochemistryRedoxSpecies rs = _db.getRedoxSpecies({name})[name];
      GeochemistryBasisSpecies bs;
      bs.name = rs.name;
      bs.radius = rs.radius;
      bs.charge = rs.charge;
      bs.molecular_weight = rs.molecular_weight;
      _basis_info.push_back(bs);
    }
    else
      mooseError(name + " does not exist in the basis species or redox species in " +
                 _db.filename());
  }
}

void
MinimalGeochemicalSystem::buildMinerals(const std::vector<std::string> & minerals)
{
  unsigned ind = 0;
  for (const auto & name : minerals)
  {
    if (_mineral_index.count(name) == 1)
      mooseError(name + " exists more than once in the minerals list");
    _mineral_index.emplace(name, ind);
    ind += 1;
    _mineral_info.push_back(_db.getMineralSpecies({name})[name]);
  }
}

void
MinimalGeochemicalSystem::buildGases(const std::vector<std::string> & gases)
{
  unsigned ind = 0;
  for (const auto & name : gases)
  {
    if (_gas_index.count(name) == 1)
      mooseError(name + " exists more than once in the gases list");
    _gas_index.emplace(name, ind);
    ind += 1;
    const GeochemistryGasSpecies gas = _db.getGasSpecies({name})[name];
    _gas_info.push_back(gas);
  }
}

void
MinimalGeochemicalSystem::buildKineticMinerals(const std::vector<std::string> & kinetic_minerals)
{
  unsigned ind = 0;
  for (const auto & name : kinetic_minerals)
  {
    if (_kinetic_mineral_index.count(name) == 1)
      mooseError(name + " exists more than once in the kinetic_minerals list");
    if (_mineral_index.count(name) == 1)
      mooseError(name + " exists in both the minerals and kinetic_minerals lists");
    _kinetic_mineral_index.emplace(name, ind);
    ind += 1;
    _kinetic_mineral_info.push_back(_db.getMineralSpecies({name})[name]);
  }
}

void
MinimalGeochemicalSystem::buildKineticRedox(const std::vector<std::string> & kinetic_redox)
{
  unsigned ind = 0;
  for (const auto & name : kinetic_redox)
  {
    if (_kinetic_redox_index.count(name) == 1)
      mooseError(name + " exists more than once in the kinetic_redox list");
    if (_basis_index.count(name) == 1)
      mooseError(name + " exists in both the basis_species and kinetic_redox lists");
    _kinetic_redox_index.emplace(name, ind);
    ind += 1;
    _kinetic_redox_info.push_back(_db.getRedoxSpecies({name})[name]);
  }
}

void
MinimalGeochemicalSystem::buildKineticSurface(
    const std::vector<std::string> & kinetic_surface_species)
{
  unsigned ind = 0;
  for (const auto & name : kinetic_surface_species)
  {
    if (_kinetic_surface_index.count(name) == 1)
      mooseError(name + " exists more than once in the kinetic_surface_species list");
    _kinetic_surface_index.emplace(name, ind);
    ind += 1;
    _kinetic_surface_info.push_back(_db.getSurfaceSpecies({name})[name]);
  }
}

void
MinimalGeochemicalSystem::buildSecondarySpecies()
{
  // run through all redox couples, including them if:
  // - they are not part of the kinetic_redox list
  // - they are not part of the basis_species list
  // - their reaction involves only basis_species or secondary species already encountered
  unsigned ind = 0;
  for (const auto & name : _db.redoxCoupleNames())
  {
    if (_kinetic_redox_index.count(name) == 0 && _basis_index.count(name) == 0)
    {
      const GeochemistryRedoxSpecies rs = _db.getRedoxSpecies({name})[name];
      // check all reaction species are in the basis
      bool all_species_in_basis_or_sec = true;
      for (const auto & element : rs.basis_species)
        if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        {
          all_species_in_basis_or_sec = false;
          break;
        }
      if (all_species_in_basis_or_sec)
      {
        _secondary_index.emplace(name, ind);
        ind += 1;
        GeochemistryEquilibriumSpecies ss;
        ss.name = rs.name;
        ss.basis_species = rs.basis_species;
        ss.equilibrium_const = rs.equilibrium_const;
        ss.radius = rs.radius;
        ss.charge = rs.charge;
        ss.molecular_weight = rs.molecular_weight;
        _secondary_info.push_back(ss);
      }
    }
  }

  // run through all secondary species, including them if:
  // - their reaction involves only basis_species, or secondary species already encountered
  for (const auto & name : _db.secondarySpeciesNames())
  {
    const GeochemistryEquilibriumSpecies ss = _db.getEquilibriumSpecies({name})[name];
    // check all reaction species are in the basis
    bool all_species_in_basis_or_sec = true;
    for (const auto & element : ss.basis_species)
      if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
      {
        all_species_in_basis_or_sec = false;
        break;
      }
    if (all_species_in_basis_or_sec)
    {
      _secondary_index.emplace(name, ind);
      ind += 1;
      _secondary_info.push_back(ss);
    }
  }

  // run through all surface species, including them if:
  // - their reaction involves only basis_species or secondary species encountered so far
  // - they are not in the kinetic_surface_species list
  for (const auto & name : _db.surfaceSpeciesNames())
  {
    if (_kinetic_surface_index.count(name) == 0)
    {
      const GeochemistrySurfaceSpecies ss = _db.getSurfaceSpecies({name})[name];
      // check all reaction species are in the basis
      bool all_species_in_basis_or_sec = true;
      for (const auto & element : ss.basis_species)
        if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        {
          all_species_in_basis_or_sec = false;
          break;
        }
      if (all_species_in_basis_or_sec)
      {
        GeochemistryEquilibriumSpecies to_sec;
        to_sec.name = ss.name;
        to_sec.basis_species = ss.basis_species;
        to_sec.radius = 0.0;
        to_sec.charge = ss.charge;
        to_sec.molecular_weight = ss.molecular_weight;
        const Real T0 = _db.getTemperatures()[0];
        for (const auto & temp : _db.getTemperatures())
          to_sec.equilibrium_const.push_back(ss.log10K + ss.dlog10KdT * (temp - T0));

        _secondary_index.emplace(name, ind);
        ind += 1;
        _secondary_info.push_back(to_sec);
      }
    }
  }
}

void
MinimalGeochemicalSystem::checkMinerals(
    const std::vector<GeochemistryMineralSpecies> & mineral_info) const
{
  for (const auto & mineral : mineral_info)
  {
    for (const auto & element : mineral.basis_species)
      if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        mooseError("The reaction for " + mineral.name + " depends on " + element.first +
                   " which is not reducable to a set of basis species");
    for (const auto & element : mineral.sorption_sites)
      if (_basis_index.count(element.first) == 0)
        mooseError("The sorbing sites for " + mineral.name + " include " + element.first +
                   " which is not in the basis_species list");
  }
}

void
MinimalGeochemicalSystem::checkGases() const
{
  for (const auto & gas : _gas_info)
    for (const auto & element : gas.basis_species)
      if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        mooseError("The reaction for " + gas.name + " depends on " + element.first +
                   " which is not reducable to a set of basis species");
}

void
MinimalGeochemicalSystem::checkKineticRedox() const
{
  for (const auto & kr : _kinetic_redox_info)
    for (const auto & element : kr.basis_species)
      if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        mooseError("The reaction for " + kr.name + " depends on " + element.first +
                   " which is not reducable to a set of basis species");
}

void
MinimalGeochemicalSystem::checkKineticSurfaceSpecies() const
{
  for (const auto & kr : _kinetic_surface_info)
    for (const auto & element : kr.basis_species)
      if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        mooseError("The reaction for " + kr.name + " depends on " + element.first +
                   " which is not reducable to a set of basis species");
}

void
MinimalGeochemicalSystem::createModel()
{
  const unsigned num_rows = _secondary_info.size() + _mineral_info.size() + _gas_info.size();
  const unsigned num_cols = _basis_info.size();
  const unsigned num_temperatures = _db.getTemperatures().size();
  unsigned ind = 0;

  _model.temperatures = _db.getTemperatures();

  // create basis_species_index map
  _model.basis_species_index = _basis_index;

  // extract the names
  _model.basis_species_name = std::vector<std::string>(num_cols);
  for (const auto & species : _basis_info)
    _model.basis_species_name[_model.basis_species_index[species.name]] = species.name;

  // initially no basis species are minerals
  _model.basis_species_mineral = std::vector<bool>(num_cols, false);

  // initially no basis species are gases
  _model.basis_species_gas = std::vector<bool>(num_cols, false);

  // record the charge
  _model.basis_species_charge = std::vector<Real>(num_cols, 0.0);
  for (const auto & species : _basis_info)
    _model.basis_species_charge[_model.basis_species_index[species.name]] = species.charge;

  // record the ionic radius
  _model.basis_species_radius = std::vector<Real>(num_cols, 0.0);
  for (const auto & species : _basis_info)
    _model.basis_species_radius[_model.basis_species_index[species.name]] = species.radius;

  // record the molecular weight
  _model.basis_species_molecular_weight = std::vector<Real>(num_cols, 0.0);
  for (const auto & species : _basis_info)
    _model.basis_species_molecular_weight[_model.basis_species_index[species.name]] =
        species.molecular_weight;

  // record the molecular weight (zero for all species except minerals)
  _model.basis_species_molecular_volume = std::vector<Real>(num_cols, 0.0);

  // the "eqm_species" stuff is rather long-winded because of the different data structures used
  // to hold secondary, mineral and gas info.  There is a bit of an overlap, however, so let's
  // create a new data structure that contains that overlapping info

  std::vector<GeochemistryEquilibriumSpecies> overlap(_secondary_info);
  for (const auto & species : _mineral_info)
  {
    GeochemistryEquilibriumSpecies es;
    es.name = species.name;
    es.molecular_weight = species.molecular_weight;
    es.equilibrium_const = species.equilibrium_const;
    es.basis_species = species.basis_species;
    overlap.push_back(es);
  }
  for (const auto & species : _gas_info)
  {
    GeochemistryEquilibriumSpecies es;
    es.name = species.name;
    es.molecular_weight = species.molecular_weight;
    es.equilibrium_const = species.equilibrium_const;
    es.basis_species = species.basis_species;
    overlap.push_back(es);
  }

  // create the eqm_species_index map
  ind = 0;
  for (const auto & species : overlap)
    _model.eqm_species_index[species.name] = ind++;

  // extract the names
  _model.eqm_species_name = std::vector<std::string>(num_rows);
  for (const auto & species : _model.eqm_species_index)
    _model.eqm_species_name[species.second] = species.first;

  // create the eqm_species_mineral vector
  _model.eqm_species_mineral = std::vector<bool>(num_rows, false);
  for (const auto & species : _mineral_info)
    _model.eqm_species_mineral[_model.eqm_species_index[species.name]] = true;
  for (const auto & species : _gas_info)
    _model.eqm_species_mineral[_model.eqm_species_index[species.name]] = false;

  // create the eqm_species_gas vector
  _model.eqm_species_gas = std::vector<bool>(num_rows, false);
  for (const auto & species : _mineral_info)
    _model.eqm_species_gas[_model.eqm_species_index[species.name]] = false;
  for (const auto & species : _gas_info)
    _model.eqm_species_gas[_model.eqm_species_index[species.name]] = true;

  // record the charge
  _model.eqm_species_charge =
      std::vector<Real>(num_rows, 0.0); // charge of gases and minerals is zero
  for (const auto & species : _secondary_info)
    _model.eqm_species_charge[_model.eqm_species_index[species.name]] = species.charge;

  // record the radius
  _model.eqm_species_radius =
      std::vector<Real>(num_rows, 0.0); // ionic radius of gases and minerals is zero
  for (const auto & species : _secondary_info)
    _model.eqm_species_radius[_model.eqm_species_index[species.name]] = species.radius;

  // record the molecular weight
  _model.eqm_species_molecular_weight = std::vector<Real>(num_rows, 0.0);
  for (const auto & species : overlap)
    _model.eqm_species_molecular_weight[_model.eqm_species_index[species.name]] =
        species.molecular_weight;

  // record the molecular volume (zero for all species except minerals)
  _model.eqm_species_molecular_volume = std::vector<Real>(num_rows, 0.0);
  for (const auto & species : _mineral_info)
    _model.eqm_species_molecular_volume[_model.eqm_species_index[species.name]] =
        species.molecular_volume;

  // record surface-complexation info
  for (const auto & species : _mineral_info)
    if (species.surface_area != 0.0)
    {
      SurfaceComplexationInfo sci;
      sci.surface_area = species.surface_area;
      sci.sorption_sites = species.sorption_sites;
      _model.surface_complexation_info[species.name] = sci;
    }
  for (const auto & species : _kinetic_mineral_info)
    if (species.surface_area != 0.0)
    {
      SurfaceComplexationInfo sci;
      sci.surface_area = species.surface_area;
      sci.sorption_sites = species.sorption_sites;
      _model.surface_complexation_info[species.name] = sci;
    }

  // record gas fugacity info
  for (const auto & species : _gas_info)
    _model.gas_chi[species.name] = species.chi;

  // create the stoichiometry matrix
  _model.eqm_stoichiometry.resize(num_rows, num_cols);
  _model.eqm_log10K.resize(num_rows, num_temperatures);

  // populate the stoichiometry
  for (const auto & species : overlap)
  {
    const unsigned row = _model.eqm_species_index[species.name];
    for (unsigned i = 0; i < num_temperatures; ++i)
      _model.eqm_log10K(row, i) = species.equilibrium_const[i];
    for (const auto & react : species.basis_species)
    {
      const Real stoi_coeff = react.second;
      if (_model.basis_species_index.count(react.first) == 1)
      {
        const unsigned col = _model.basis_species_index[react.first];
        _model.eqm_stoichiometry(row, col) += react.second;
      }
      else if (_secondary_index.count(react.first) == 1)
      {
        // reaction species is not a basis component, but a secondary component.
        // So express stoichiometry in terms of the secondary component's reaction
        const unsigned sec_row = _model.eqm_species_index[react.first];
        for (unsigned i = 0; i < num_temperatures; ++i)
          _model.eqm_log10K(row, i) += stoi_coeff * _model.eqm_log10K(sec_row, i);
        for (unsigned col = 0; col < num_cols; ++col)
          _model.eqm_stoichiometry(row, col) += stoi_coeff * _model.eqm_stoichiometry(sec_row, col);
      }
      else
        mooseError("Species " + species.name + " includes " + react.first +
                   ", which cannot be expressed in terms of the basis.  Previous checks must be "
                   "erroneous!");
    }
  }

  // To build the kin_species_index, kin_species_name, etc, let's build an "overlap", similar to
  // above
  std::vector<GeochemistryMineralSpecies> overlap_kin(_kinetic_mineral_info);
  for (const auto & species : _kinetic_redox_info)
  {
    GeochemistryMineralSpecies ms;
    ms.name = species.name;
    ms.molecular_volume = 0.0;
    ms.basis_species = species.basis_species;
    ms.molecular_weight = species.molecular_weight;
    overlap_kin.push_back(ms);
  }
  for (const auto & species : _kinetic_surface_info)
  {
    GeochemistryMineralSpecies ms;
    ms.name = species.name;
    ms.molecular_volume = 0.0;
    ms.basis_species = species.basis_species;
    ms.molecular_weight = species.molecular_weight;
    overlap_kin.push_back(ms);
  }
  const unsigned num_kin = overlap_kin.size();

  // create the kin_species_index map
  ind = 0;
  for (const auto & species : overlap_kin)
    _model.kin_species_index[species.name] = ind++;

  // extract the names
  _model.kin_species_name = std::vector<std::string>(num_kin);
  for (const auto & species : _model.kin_species_index)
    _model.kin_species_name[species.second] = species.first;

  // build the kin_species_mineral info
  _model.kin_species_mineral = std::vector<bool>(num_kin, true);
  for (const auto & species : _kinetic_redox_info)
    _model.kin_species_mineral[_model.kin_species_index[species.name]] = false;
  for (const auto & species : _kinetic_surface_info)
    _model.kin_species_mineral[_model.kin_species_index[species.name]] = false;

  // build the kin_species_charge info
  _model.kin_species_charge = std::vector<Real>(num_kin, 0.0);
  for (const auto & species : _kinetic_redox_info)
    _model.kin_species_charge[_model.kin_species_index[species.name]] = species.charge;
  for (const auto & species : _kinetic_surface_info)
    _model.kin_species_charge[_model.kin_species_index[species.name]] = species.charge;

  // extract the molecular weight
  _model.kin_species_molecular_weight = std::vector<Real>(num_kin, 0.0);
  for (const auto & species : overlap_kin)
    _model.kin_species_molecular_weight[_model.kin_species_index[species.name]] =
        species.molecular_weight;

  // extract the molecular volume
  _model.kin_species_molecular_volume = std::vector<Real>(num_kin, 0.0);
  for (const auto & species : overlap_kin)
    _model.kin_species_molecular_volume[_model.kin_species_index[species.name]] =
        species.molecular_volume;

  // extract the stoichiometry
  _model.kin_stoichiometry.resize(num_kin, num_cols);

  // populate the stoichiometry
  for (const auto & species : overlap_kin)
  {
    const unsigned row = _model.kin_species_index[species.name];
    for (const auto & react : species.basis_species)
    {
      const Real stoi_coeff = react.second;
      if (_model.basis_species_index.count(react.first) == 1)
      {
        const unsigned col = _model.basis_species_index[react.first];
        _model.kin_stoichiometry(row, col) += react.second;
      }
      else if (_model.eqm_species_index.count(react.first) == 1)
      {
        // reaction species is not a basis component, but a secondary component.
        // So express stoichiometry in terms of the secondary component's reaction
        const unsigned sec_row = _model.eqm_species_index[react.first];
        for (unsigned col = 0; col < num_cols; ++col)
          _model.kin_stoichiometry(row, col) += stoi_coeff * _model.eqm_stoichiometry(sec_row, col);
      }
      else
        mooseError("Kinetic species " + species.name + " includes " + react.first +
                   ", which cannot be expressed in terms of the basis.  Previous checks must be "
                   "erroneous!");
    }
  }
}

ModelGeochemicalDatabase
MinimalGeochemicalSystem::modelGeochemicalDatabaseCopy() const
{
  ModelGeochemicalDatabase mgd = _model;
  return mgd;
}
