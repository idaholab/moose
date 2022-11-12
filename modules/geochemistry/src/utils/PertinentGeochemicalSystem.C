//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PertinentGeochemicalSystem.h"

PertinentGeochemicalSystem::PertinentGeochemicalSystem(
    const GeochemicalDatabaseReader & db,
    const std::vector<std::string> & basis_species,
    const std::vector<std::string> & minerals,
    const std::vector<std::string> & gases,
    const std::vector<std::string> & kinetic_minerals,
    const std::vector<std::string> & kinetic_redox,
    const std::vector<std::string> & kinetic_surface_species,
    const std::string & redox_ox,
    const std::string & redox_e)
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
    _redox_ox(redox_ox),
    _redox_e(redox_e),
    _model(_db)
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

  // Build minerals in the case that minerals = {"*"}
  buildAllMinerals(minerals);

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

unsigned
PertinentGeochemicalSystem::getIndexOfOriginalBasisSpecies(const std::string & name) const
{
  try
  {
    return _basis_index.at(name);
  }
  catch (const std::out_of_range &)
  {
    mooseError("species ", name, " is not in the original basis");
  }
  catch (...)
  {
    throw;
  }
}

std::vector<std::string>
PertinentGeochemicalSystem::originalBasisNames() const
{
  std::vector<std::string> names(_basis_info.size());
  for (const auto & name_ind : _basis_index)
    names[name_ind.second] = name_ind.first;
  return names;
}

void
PertinentGeochemicalSystem::buildBasis(const std::vector<std::string> & basis_species)
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
PertinentGeochemicalSystem::buildMinerals(const std::vector<std::string> & minerals)
{
  unsigned ind = 0;
  if (minerals.size() == 1 && minerals[0] == "*")
    return; // buildAllMinerals is called later
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
PertinentGeochemicalSystem::buildGases(const std::vector<std::string> & gases)
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
PertinentGeochemicalSystem::buildKineticMinerals(const std::vector<std::string> & kinetic_minerals)
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
PertinentGeochemicalSystem::buildKineticRedox(const std::vector<std::string> & kinetic_redox)
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
PertinentGeochemicalSystem::buildKineticSurface(
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
PertinentGeochemicalSystem::buildSecondarySpecies()
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
  // - the name is not _redox_e (which is usually "e-" the free electron)
  for (const auto & name : _db.secondarySpeciesNames())
  {
    if (name == _redox_e)
      continue;
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
        to_sec.radius = -1.5; // flag to activity calculators that activity coefficient = 1
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
PertinentGeochemicalSystem::buildAllMinerals(const std::vector<std::string> & minerals)
{
  if (!(minerals.size() == 1 && minerals[0] == "*"))
    return; // buildMinerals has done its job of building _mineral_info and _mineral_index
  unsigned ind = 0;
  for (const auto & name_ms : _db.getMineralSpecies(_db.mineralSpeciesNames()))
  {
    if (_kinetic_mineral_index.count(name_ms.first) == 1)
      continue;
    bool known_basis_only = true;
    for (const auto & basis_stoi : name_ms.second.basis_species)
    {
      if (_basis_index.count(basis_stoi.first) == 0 &&
          _secondary_index.count(basis_stoi.first) == 0)
      {
        known_basis_only = false;
        break;
      }
    }
    if (known_basis_only)
    {
      _mineral_index.emplace(name_ms.first, ind);
      ind += 1;
      _mineral_info.push_back(name_ms.second);
    }
  }
}

bool
PertinentGeochemicalSystem::checkRedoxe()
{
  bool found = false;
  for (const auto & name : _db.secondarySpeciesNames())
    if (name == _redox_e)
    {
      found = true;
      const GeochemistryEquilibriumSpecies ss = _db.getEquilibriumSpecies({name})[name];
      for (const auto & element : ss.basis_species)
        if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
          return false;
    }
  return found;
}

void
PertinentGeochemicalSystem::buildRedoxeInfo(std::vector<Real> & redox_e_stoichiometry,
                                            std::vector<Real> & redox_e_log10K)
{
  const unsigned num_basis = _basis_info.size();
  const unsigned numT = _db.getTemperatures().size();
  redox_e_stoichiometry.assign(num_basis, 0.0);
  redox_e_log10K.assign(numT, 0.0);
  for (const auto & name : _db.secondarySpeciesNames())
    if (name == _redox_e)
    {
      const GeochemistryEquilibriumSpecies ss = _db.getEquilibriumSpecies({name})[name];
      for (unsigned i = 0; i < numT; ++i)
        redox_e_log10K[i] = ss.equilibrium_const[i];
      for (const auto & react : ss.basis_species)
      {
        const Real stoi_coeff = react.second;
        if (_model.basis_species_index.count(react.first) == 1)
        {
          const unsigned col = _model.basis_species_index[react.first];
          redox_e_stoichiometry[col] += react.second;
        }
        else if (_secondary_index.count(react.first) == 1)
        {
          // reaction species is not a basis component, but a secondary component.
          // So express stoichiometry in terms of the secondary component's reaction
          const unsigned sec_row = _model.eqm_species_index[react.first];
          for (unsigned i = 0; i < numT; ++i)
            redox_e_log10K[i] += stoi_coeff * _model.eqm_log10K(sec_row, i);
          for (unsigned col = 0; col < num_basis; ++col)
            redox_e_stoichiometry[col] += stoi_coeff * _model.eqm_stoichiometry(sec_row, col);
        }
      }
    }
}

void
PertinentGeochemicalSystem::checkMinerals(
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
PertinentGeochemicalSystem::checkGases() const
{
  for (const auto & gas : _gas_info)
    for (const auto & element : gas.basis_species)
      if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        mooseError("The reaction for " + gas.name + " depends on " + element.first +
                   " which is not reducable to a set of basis species");
}

void
PertinentGeochemicalSystem::checkKineticRedox() const
{
  for (const auto & kr : _kinetic_redox_info)
    for (const auto & element : kr.basis_species)
      if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        mooseError("The reaction for " + kr.name + " depends on " + element.first +
                   " which is not reducable to a set of basis species");
}

void
PertinentGeochemicalSystem::checkKineticSurfaceSpecies() const
{
  for (const auto & kr : _kinetic_surface_info)
    for (const auto & element : kr.basis_species)
      if (_basis_index.count(element.first) == 0 && _secondary_index.count(element.first) == 0)
        mooseError("The reaction for " + kr.name + " depends on " + element.first +
                   " which is not reducable to a set of basis species");
}

void
PertinentGeochemicalSystem::createModel()
{
  const unsigned num_rows = _secondary_info.size() + _mineral_info.size() + _gas_info.size();
  const unsigned num_cols = _basis_info.size();
  const unsigned num_temperatures = _db.getTemperatures().size();
  unsigned ind = 0;

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

  // initially all basis species are involved in transport (this gets modified for surface species
  // below)
  _model.basis_species_transported = std::vector<bool>(num_cols, true);

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

  // create the eqm_species_transported vector (true for non-minerals) - gets modified below for
  // surface species
  _model.eqm_species_transported = std::vector<bool>(num_rows, true);
  for (const auto & species : _mineral_info)
    _model.eqm_species_transported[_model.eqm_species_index.at(species.name)] = false;

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

  // Build the redox information, if any.  Here we express any O2(aq) in the redox equations in
  // terms of redox_e (which is usually e-)
  _model.redox_lhs = _redox_e;
  std::vector<Real> redox_e_stoichiometry(num_cols, 0.0);
  std::vector<Real> redox_e_log10K(num_temperatures, 0.0);
  std::vector<Real> redox_stoi;
  std::vector<Real> redox_log10K;
  if ((_model.basis_species_index.count(_redox_ox) == 1) && checkRedoxe())
  {
    // construct the stoichiometry and log10K for _redox_e and put it
    buildRedoxeInfo(redox_e_stoichiometry, redox_e_log10K);
    redox_stoi.insert(redox_stoi.end(), redox_e_stoichiometry.begin(), redox_e_stoichiometry.end());
    redox_log10K.insert(redox_log10K.end(), redox_e_log10K.begin(), redox_e_log10K.end());
    // the electron reaction is
    // e- = nuw_i * basis_i + beta * O2(aq), where we've pulled out the O2(aq) because it's
    // special
    const unsigned o2_index = _model.basis_species_index.at(_redox_ox);
    const Real beta = redox_e_stoichiometry[o2_index];
    if (beta != 0.0)
    {
      for (const auto & bs : _model.basis_species_index)
        if (_db.isRedoxSpecies(bs.first))
        {
          // this basis species is a redox couple in disequilibrium
          const GeochemistryRedoxSpecies rs = _db.getRedoxSpecies({bs.first})[bs.first];
          // check that its reaction involves only basis species, and record the stoichiometry in
          // the current basis
          std::vector<Real> stoi(num_cols, 0.0);
          bool only_involves_basis_species = true;
          for (const auto & name_stoi : rs.basis_species)
          {
            if (_model.basis_species_index.count(name_stoi.first) == 1)
              stoi[_model.basis_species_index.at(name_stoi.first)] = name_stoi.second;
            else
            {
              only_involves_basis_species = false;
              break;
            }
          }
          if (!only_involves_basis_species)
            continue;
          // Reaction is now
          // redox = nu_i * basis_i + alpha * O2(aq), where we've pulled the O2(aq) out because
          // it's special. Now pull the redox couple to the RHS of the reaction, so we have 0 =
          // -redox + nu_i * basis_i + alpha * O2(aq)
          stoi[bs.second] = -1.0;
          // check that the stoichiometry involves O2(aq)
          const Real alpha = stoi[o2_index];
          if (alpha == 0.0)
            continue;
          // multiply equation -beta/alpha so it reads
          // 0 = -beta/alpha * (-redox + nu_i * basis_i) - beta * O2(aq)
          for (unsigned basis_i = 0; basis_i < num_cols; ++basis_i)
            stoi[basis_i] *= -beta / alpha;
          // add the equation to e- = nuw_i * basis_i + beta * O2(aq)
          for (unsigned basis_i = 0; basis_i < num_cols; ++basis_i)
            stoi[basis_i] += redox_e_stoichiometry[basis_i];
          // now the reation is e- = nuw_i * basis_i - beta/alpha * (-redox + nu_i * basis_i)
          redox_stoi.insert(redox_stoi.end(), stoi.begin(), stoi.end());

          // record the equilibrium constants
          for (unsigned temp = 0; temp < num_temperatures; ++temp)
            redox_log10K.push_back((-beta / alpha) * rs.equilibrium_const[temp] +
                                   redox_e_log10K[temp]);
        }
    }
  }
  // record the above in the model.redox_stoichiometry and model.redox_log10K DenseMatrices
  const unsigned num_redox = redox_stoi.size() / num_cols;
  _model.redox_stoichiometry.resize(num_redox, num_cols);
  for (unsigned red = 0; red < num_redox; ++red)
    for (unsigned basis_i = 0; basis_i < num_cols; ++basis_i)
      _model.redox_stoichiometry(red, basis_i) = redox_stoi[red * num_cols + basis_i];
  _model.redox_log10K.resize(num_redox, num_temperatures);
  for (unsigned red = 0; red < num_redox; ++red)
    for (unsigned temp = 0; temp < num_temperatures; ++temp)
      _model.redox_log10K(red, temp) = redox_log10K[red * num_temperatures + temp];

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
    ms.equilibrium_const = species.equilibrium_const;
    overlap_kin.push_back(ms);
  }
  for (const auto & species : _kinetic_surface_info)
  {
    GeochemistryMineralSpecies ms;
    ms.name = species.name;
    ms.molecular_volume = 0.0;
    ms.basis_species = species.basis_species;
    ms.molecular_weight = species.molecular_weight;
    const Real T0 = _db.getTemperatures()[0];
    for (const auto & temp : _db.getTemperatures())
      ms.equilibrium_const.push_back(species.log10K + species.dlog10KdT * (temp - T0));
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

  // create the kin_species_transported vector (false for minerals and surface species)
  _model.kin_species_transported = std::vector<bool>(num_kin, true);
  for (const auto & species : _kinetic_mineral_info)
    _model.kin_species_transported[_model.kin_species_index.at(species.name)] = false;
  for (const auto & species : _kinetic_surface_info)
    _model.kin_species_transported[_model.kin_species_index.at(species.name)] = false;

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
  _model.kin_log10K.resize(num_kin, num_temperatures);

  // populate the stoichiometry
  for (const auto & species : overlap_kin)
  {
    const unsigned row = _model.kin_species_index[species.name];
    for (unsigned i = 0; i < num_temperatures; ++i)
      _model.kin_log10K(row, i) = species.equilibrium_const[i];
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
        for (unsigned i = 0; i < num_temperatures; ++i)
          _model.kin_log10K(row, i) += stoi_coeff * _model.eqm_log10K(sec_row, i);
        for (unsigned col = 0; col < num_cols; ++col)
          _model.kin_stoichiometry(row, col) += stoi_coeff * _model.eqm_stoichiometry(sec_row, col);
      }
      else
        mooseError("Kinetic species " + species.name + " includes " + react.first +
                   ", which cannot be expressed in terms of the basis.  Previous checks must be "
                   "erroneous!");
    }
  }

  // check that there are no repeated sorbing sites in the SurfaceComplexationInfo
  std::vector<std::string> all_sorbing_sites;
  for (const auto & name_info : _model.surface_complexation_info)
    for (const auto & name_frac : name_info.second.sorption_sites)
      if (std::find(all_sorbing_sites.begin(), all_sorbing_sites.end(), name_frac.first) !=
          all_sorbing_sites.end())
        mooseError(
            "The sorbing site ", name_frac.first, " appears in more than one sorbing mineral");
      else
        all_sorbing_sites.push_back(name_frac.first);

  // build the information related to surface sorption, and modify the species_transported vectors
  _model.surface_sorption_related.assign(num_rows, false);
  _model.surface_sorption_number.assign(num_rows, 99);
  for (const auto & name_info :
       _model.surface_complexation_info) // all minerals involved in surface complexation
  {
    for (const auto & name_frac :
         name_info.second.sorption_sites) // all sorption sites on the given mineral
    {
      const unsigned basis_index_of_sorption_site = _model.basis_species_index.at(name_frac.first);
      _model.basis_species_transported[basis_index_of_sorption_site] = false;
    }
    bool mineral_involved_in_eqm = false;
    for (const auto & name_frac :
         name_info.second.sorption_sites) // all sorption sites on the given mineral
    {
      const unsigned basis_index_of_sorption_site = _model.basis_species_index.at(name_frac.first);
      for (unsigned j = 0; j < num_rows; ++j) // all equilibrium species
        if (_model.eqm_stoichiometry(j, basis_index_of_sorption_site) != 0.0)
        {
          mineral_involved_in_eqm = true;
          break;
        }
    }
    if (!mineral_involved_in_eqm)
      continue;
    const unsigned num_surface_sorption = _model.surface_sorption_name.size();
    _model.surface_sorption_name.push_back(name_info.first);
    _model.surface_sorption_area.push_back(name_info.second.surface_area);
    for (const auto & name_frac :
         name_info.second.sorption_sites) // all sorption sites on the given mineral
    {
      const unsigned basis_index_of_sorption_site = _model.basis_species_index.at(name_frac.first);
      for (unsigned j = 0; j < num_rows; ++j) // all equilibrium species
        if (_model.eqm_stoichiometry(j, basis_index_of_sorption_site) != 0.0)
        {
          if (_model.surface_sorption_related[j])
            mooseError("It is an error for any equilibrium species (such as ",
                       _model.eqm_species_name[j],
                       ") to have a reaction involving more than one sorbing site");
          _model.surface_sorption_related[j] = true;
          _model.surface_sorption_number[j] = num_surface_sorption;
          _model.eqm_species_transported[j] = false;
        }
    }
  }
}

const ModelGeochemicalDatabase &
PertinentGeochemicalSystem::modelGeochemicalDatabase() const
{
  return _model;
}

void
PertinentGeochemicalSystem::addKineticRate(const KineticRateUserDescription & description)
{
  const std::string kinetic_species = description.kinetic_species_name;
  if (_model.kin_species_index.count(kinetic_species) == 0)
    mooseError("Cannot prescribe a kinetic rate to species ",
               kinetic_species,
               " since it is not a kinetic species");
  const unsigned kinetic_species_index = _model.kin_species_index.at(kinetic_species);

  // build the promoting index list
  const unsigned num_pro = description.promoting_species.size();
  const unsigned num_basis = _model.basis_species_name.size();
  const unsigned num_eqm = _model.eqm_species_name.size();
  std::vector<Real> promoting_ind(num_basis + num_eqm, 0.0);
  std::vector<Real> promoting_m_ind(num_basis + num_eqm, 0.0);
  std::vector<Real> promoting_k(num_basis + num_eqm, 0.0);
  for (unsigned i = 0; i < num_pro; ++i)
  {
    unsigned index = 0;
    const std::string promoting_species = description.promoting_species[i];
    if (_model.basis_species_index.count(promoting_species) == 1)
      index = _model.basis_species_index.at(promoting_species);
    else if (_model.eqm_species_index.count(promoting_species) == 1)
      index = num_basis + _model.eqm_species_index.at(promoting_species);
    else
      mooseError(
          "Promoting species ", promoting_species, " must be a basis or a secondary species");
    promoting_ind[index] = description.promoting_indices[i];
    promoting_m_ind[index] = description.promoting_monod_indices[i];
    promoting_k[index] = description.promoting_half_saturation[i];
  }
  unsigned progeny_num = 0;
  if (_model.basis_species_index.count(description.progeny) == 1)
    progeny_num = _model.basis_species_index.at(description.progeny);
  else if (_model.eqm_species_index.count(description.progeny) == 1)
    progeny_num = num_basis + _model.eqm_species_index.at(description.progeny);
  else
    mooseError("Progeny ", description.progeny, " must be a basis or a secondary species");

  // append the result to kin_rate
  _model.kin_rate.push_back(KineticRateDefinition(kinetic_species_index,
                                                  promoting_ind,
                                                  promoting_m_ind,
                                                  promoting_k,
                                                  progeny_num,
                                                  description));
}
