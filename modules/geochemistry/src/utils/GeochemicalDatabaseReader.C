//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalDatabaseReader.h"
#include "GeochemicalDatabaseValidator.h"

#include "MooseUtils.h"
#include "Conversion.h"
#include "string"
#include <fstream>

GeochemicalDatabaseReader::GeochemicalDatabaseReader(
    const FileName filename,
    const bool reexpress_free_electron,
    const bool use_piecewise_interpolation,
    const bool remove_all_extrapolated_secondary_species)
  : _filename(filename)
{
  read(_filename);
  validate(_filename, _root);

  if (reexpress_free_electron)
    reexpressFreeElectron();

  if (use_piecewise_interpolation && _root["Header"].contains("logk model"))
    _root["Header"]["logk model"] = "piecewise-linear";

  if (remove_all_extrapolated_secondary_species)
    removeExtrapolatedSecondarySpecies();

  setTemperatures();
  setDebyeHuckel();
  setNeutralSpeciesActivity();
}

void
GeochemicalDatabaseReader::read(const FileName filename)
{
  MooseUtils::checkFileReadable(filename);

  // Read the JSON database
  std::ifstream jsondata(filename);
  jsondata >> _root;
}

void
GeochemicalDatabaseReader::validate(const FileName filename, const nlohmann::json & db)
{
  // Validate the JSON database so that we don't have to check array sizes,
  // check for conversion issues, etc when extracting data using get methods
  GeochemicalDatabaseValidator dbv(filename, db);
  dbv.validate();
}

void
GeochemicalDatabaseReader::reexpressFreeElectron()
{
  if (!_root.contains("free electron") || !_root["free electron"].contains("e-") ||
      !_root["free electron"]["e-"]["species"].contains("O2(g)"))
    return;
  if (!_root.contains("basis species") || !_root["basis species"].contains("O2(aq)"))
    return;
  if (!_root.contains("gas species") || !_root["gas species"].contains("O2(g)") ||
      !_root["gas species"]["O2(g)"]["species"].contains("O2(aq)") ||
      (_root["gas species"]["O2(g)"]["species"].size() != 1))
    return;

  // remove O2(g) in the "e-" and replace with O2(aq)
  const std::string stoi_o2g =
      nlohmann::to_string(_root["free electron"]["e-"]["species"]["O2(g)"]);
  _root["free electron"]["e-"]["species"].erase("O2(g)");
  _root["free electron"]["e-"]["species"]["O2(aq)"] = stoi_o2g;
  const Real stoi = getReal(stoi_o2g);

  // alter equilibrium constants
  if (!_root["Header"].contains("temperatures"))
    return;
  for (unsigned i = 0; i < _root["Header"]["temperatures"].size(); ++i)
  {
    const Real logk_e = getReal(_root["free electron"]["e-"]["logk"][i]);
    const Real logk_o2 = getReal(_root["gas species"]["O2(g)"]["logk"][i]);
    const Real newk = logk_e + stoi * logk_o2;
    _root["free electron"]["e-"]["logk"][i] = std::to_string(newk);
  }
}

void
GeochemicalDatabaseReader::removeExtrapolatedSecondarySpecies()
{
  if (_root.contains("secondary species"))
  {
    std::set<std::string> remove; // items to remove
    for (const auto & item : _root["secondary species"].items())
      if (item.value().contains("note"))
        remove.insert(item.key());

    for (const auto & name : remove)
      _root["secondary species"].erase(name);
  }
}

std::string
GeochemicalDatabaseReader::getActivityModel() const
{
  return _root["Header"]["activity model"];
}

std::string
GeochemicalDatabaseReader::getFugacityModel() const
{
  return _root["Header"]["fugacity model"];
}

std::string
GeochemicalDatabaseReader::getLogKModel() const
{
  return _root["Header"]["logk model"];
}

void
GeochemicalDatabaseReader::setTemperatures()
{
  if (_root["Header"].contains("temperatures"))
  {
    auto temperatures = _root["Header"]["temperatures"];
    _temperature_points.resize(temperatures.size());
    for (unsigned int i = 0; i < temperatures.size(); ++i)
      _temperature_points[i] = getReal(temperatures[i]);
  }
}

const std::vector<Real> &
GeochemicalDatabaseReader::getTemperatures() const
{
  return _temperature_points;
}

std::vector<Real>
GeochemicalDatabaseReader::getPressures()
{
  // Read pressure points
  if (_root["Header"].contains("pressures"))
  {
    auto pressures = _root["Header"]["pressures"];
    _pressure_points.resize(pressures.size());
    for (unsigned int i = 0; i < pressures.size(); ++i)
      _pressure_points[i] = getReal(pressures[i]);
  }

  return _pressure_points;
}

void
GeochemicalDatabaseReader::setDebyeHuckel()
{
  if (getActivityModel() == "debye-huckel")
  {
    if (_root["Header"].contains("adh"))
    {
      std::vector<Real> adhvals(_root["Header"]["adh"].size());
      for (unsigned int i = 0; i < _root["Header"]["adh"].size(); ++i)
        adhvals[i] = getReal(_root["Header"]["adh"][i]);
      _debye_huckel.adh = adhvals;
    }

    if (_root["Header"].contains("bdh"))
    {
      std::vector<Real> bdhvals(_root["Header"]["bdh"].size());
      for (unsigned int i = 0; i < _root["Header"]["bdh"].size(); ++i)
        bdhvals[i] = getReal(_root["Header"]["bdh"][i]);
      _debye_huckel.bdh = bdhvals;
    }

    if (_root["Header"].contains("bdot"))
    {
      std::vector<Real> bdotvals(_root["Header"]["bdot"].size());
      for (unsigned int i = 0; i < _root["Header"]["bdot"].size(); ++i)
        bdotvals[i] = getReal(_root["Header"]["bdot"][i]);
      _debye_huckel.bdot = bdotvals;
    }
  }
}

const GeochemistryDebyeHuckel &
GeochemicalDatabaseReader::getDebyeHuckel() const
{
  if (getActivityModel() != "debye-huckel")
    mooseError("Attempted to get Debye-Huckel activity parameters but the activity model is ",
               getActivityModel());
  return _debye_huckel;
}

std::map<std::string, GeochemistryElements>
GeochemicalDatabaseReader::getElements()
{
  if (_root.contains("elements"))
  {
    for (auto & el : _root["elements"].items())
    {
      _elements[el.key()].name = el.value()["name"];
      _elements[el.key()].molecular_weight = getReal(el.value()["molecular weight"]);
    }
  }

  return _elements;
}

std::map<std::string, GeochemistryBasisSpecies>
GeochemicalDatabaseReader::getBasisSpecies(const std::vector<std::string> & names)
{
  // Parse the basis species specified in names
  for (const auto & species : names)
    if (_root["basis species"].contains(species))
    {
      GeochemistryBasisSpecies dbs;

      auto basis_species = _root["basis species"][species];
      dbs.name = species;
      dbs.radius = getReal(basis_species["radius"]);
      dbs.charge = getReal(basis_species["charge"]);
      dbs.molecular_weight = getReal(basis_species["molecular weight"]);

      std::map<std::string, Real> elements;
      for (auto & el : basis_species["elements"].items())
        elements[el.key()] = getReal(el.value());

      dbs.elements = elements;

      _basis_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _basis_species;
}

std::map<std::string, GeochemistryEquilibriumSpecies>
GeochemicalDatabaseReader::getEquilibriumSpecies(const std::vector<std::string> & names)
{
  // Parse the secondary species specified in names
  for (const auto & species : names)
    if (_root["secondary species"].contains(species) or _root["free electron"].contains(species))
    {
      GeochemistryEquilibriumSpecies dbs;

      auto sec_species = _root["secondary species"].contains(species)
                             ? _root["secondary species"][species]
                             : _root["free electron"][species];
      dbs.name = species;
      dbs.radius = getReal(sec_species["radius"]);
      dbs.charge = getReal(sec_species["charge"]);
      dbs.molecular_weight = getReal(sec_species["molecular weight"]);

      std::vector<Real> eq_const(sec_species["logk"].size());
      for (unsigned int i = 0; i < sec_species["logk"].size(); ++i)
        eq_const[i] = getReal(sec_species["logk"][i]);

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : sec_species["species"].items())
        basis_species[bs.key()] = getReal(bs.value());

      dbs.basis_species = basis_species;

      _equilibrium_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _equilibrium_species;
}

std::map<std::string, GeochemistryMineralSpecies>
GeochemicalDatabaseReader::getMineralSpecies(const std::vector<std::string> & names)
{
  // Parse the mineral species specified in names
  for (const auto & species : names)
    if (_root["mineral species"].contains(species))
    {
      GeochemistryMineralSpecies dbs;

      auto mineral_species = _root["mineral species"][species];
      dbs.name = species;
      dbs.molecular_weight = getReal(mineral_species["molecular weight"]);
      dbs.molecular_volume = getReal(mineral_species["molar volume"]);

      std::vector<Real> eq_const(mineral_species["logk"].size());
      for (unsigned int i = 0; i < mineral_species["logk"].size(); ++i)
        eq_const[i] = getReal(mineral_species["logk"][i]);

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : mineral_species["species"].items())
        basis_species[bs.key()] = getReal(bs.value());

      dbs.basis_species = basis_species;

      // recover sorption information, if any
      std::map<std::string, Real> species_and_sorbing_density;
      dbs.surface_area = 0.0;
      if (_root["sorbing minerals"].contains(species))
      {
        auto sorbing_mineral = _root["sorbing minerals"][species];
        dbs.surface_area = getReal(sorbing_mineral["surface area"]);

        for (auto & site : sorbing_mineral["sorbing sites"].items())
          species_and_sorbing_density[site.key()] = getReal(site.value());
      }
      dbs.sorption_sites = species_and_sorbing_density;

      _mineral_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _mineral_species;
}

std::map<std::string, GeochemistryGasSpecies>
GeochemicalDatabaseReader::getGasSpecies(const std::vector<std::string> & names)
{
  // Parse the gas species specified in names
  for (const auto & species : names)
    if (_root["gas species"].contains(species))
    {
      GeochemistryGasSpecies dbs;

      auto gas_species = _root["gas species"][species];
      dbs.name = species;
      dbs.molecular_weight = getReal(gas_species["molecular weight"]);

      std::vector<Real> eq_const(gas_species["logk"].size());
      for (unsigned int i = 0; i < gas_species["logk"].size(); ++i)
        eq_const[i] = getReal(gas_species["logk"][i]);

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : gas_species["species"].items())
        basis_species[bs.key()] = getReal(bs.value());

      dbs.basis_species = basis_species;

      // Optional fugacity coefficients
      if (gas_species.contains("chi"))
      {
        std::vector<Real> chi(gas_species["chi"].size());
        for (unsigned int i = 0; i < gas_species["chi"].size(); ++i)
          chi[i] = getReal(gas_species["chi"][i]);

        dbs.chi = chi;
      }

      if (gas_species.contains("Pcrit"))
        dbs.Pcrit = getReal(gas_species["Pcrit"]);

      if (gas_species.contains("Tcrit"))
        dbs.Tcrit = getReal(gas_species["Tcrit"]);

      if (gas_species.contains("omega"))
        dbs.omega = getReal(gas_species["omega"]);

      _gas_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _gas_species;
}

std::map<std::string, GeochemistryRedoxSpecies>
GeochemicalDatabaseReader::getRedoxSpecies(const std::vector<std::string> & names)
{
  // Parse the redox species specified in names
  for (const auto & species : names)
    if (_root["redox couples"].contains(species))
    {
      GeochemistryRedoxSpecies dbs;

      auto redox_species = _root["redox couples"][species];
      dbs.name = species;
      dbs.radius = getReal(redox_species["radius"]);
      dbs.charge = getReal(redox_species["charge"]);
      dbs.molecular_weight = getReal(redox_species["molecular weight"]);

      std::vector<Real> eq_const(redox_species["logk"].size());
      for (unsigned int i = 0; i < redox_species["logk"].size(); ++i)
        eq_const[i] = getReal(redox_species["logk"][i]);

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : redox_species["species"].items())
        basis_species[bs.key()] = getReal(bs.value());

      dbs.basis_species = basis_species;

      _redox_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _redox_species;
}

std::map<std::string, GeochemistryOxideSpecies>
GeochemicalDatabaseReader::getOxideSpecies(const std::vector<std::string> & names)
{
  // Parse the oxide species specified in names
  for (auto & species : names)
    if (_root["oxides"].contains(species))
    {
      GeochemistryOxideSpecies dbs;

      auto oxide_species = _root["oxides"][species];
      dbs.name = species;
      dbs.molecular_weight = getReal(oxide_species["molecular weight"]);

      std::map<std::string, Real> basis_species;
      for (auto & bs : oxide_species["species"].items())
        basis_species[bs.key()] = getReal(bs.value());

      dbs.basis_species = basis_species;

      _oxide_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _oxide_species;
}

std::map<std::string, GeochemistrySurfaceSpecies>
GeochemicalDatabaseReader::getSurfaceSpecies(const std::vector<std::string> & names)
{
  // Parse the secondary species specified in names
  for (const auto & species : names)
    if (_root["surface species"].contains(species))
    {
      GeochemistrySurfaceSpecies dbs;

      auto surface_species = _root["surface species"][species];
      dbs.name = species;
      dbs.charge = getReal(surface_species["charge"]);
      dbs.molecular_weight = getReal(surface_species["molecular weight"]);
      dbs.log10K = getReal(surface_species["log K"]);
      dbs.dlog10KdT = getReal(surface_species["dlogK/dT"]);

      std::map<std::string, Real> basis_species;
      for (auto & bs : surface_species["species"].items())
        basis_species[bs.key()] = getReal(bs.value());

      dbs.basis_species = basis_species;

      _surface_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _surface_species;
}

void
GeochemicalDatabaseReader::setNeutralSpeciesActivity()
{
  if (_root["Header"].contains("neutral species"))
  {
    auto neutral_species = _root["Header"]["neutral species"];
    for (auto & ns : neutral_species.items())
    {
      std::vector<std::vector<Real>> coeffs;

      for (auto & nsac : ns.value().items())
      {
        if (nsac.key() == "note")
          continue;
        std::vector<Real> coeffvec(nsac.value().size());

        for (unsigned int i = 0; i < coeffvec.size(); ++i)
          coeffvec[i] = getReal(nsac.value()[i]);

        coeffs.push_back(coeffvec);
      }

      // GeochemistryNeutralSpeciesActivity expects four vectos, so
      // add empty vectors if coeffs.size() != 4
      coeffs.resize(4, {});

      GeochemistryNeutralSpeciesActivity nsa(coeffs);

      _neutral_species_activity[ns.key()] = nsa;
    }
  }
}

const std::map<std::string, GeochemistryNeutralSpeciesActivity> &
GeochemicalDatabaseReader::getNeutralSpeciesActivity() const
{
  if (!_root["Header"].contains("neutral species"))
    mooseError("No neutral species activity coefficients in database");
  return _neutral_species_activity;
}

std::vector<std::string>
GeochemicalDatabaseReader::equilibriumReactions(const std::vector<std::string> & names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    auto species = names[i];

    if (_root["secondary species"].contains(species))
    {
      auto sec_species = _root["secondary species"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : sec_species["species"].items())
        this_basis_species[bs.key()] = getReal(bs.value());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::mineralReactions(const std::vector<std::string> & names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    const auto species = names[i];

    if (_root["mineral species"].contains(species))
    {
      auto min_species = _root["mineral species"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : min_species["species"].items())
        this_basis_species[bs.key()] = getReal(bs.value());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::gasReactions(const std::vector<std::string> & names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    const auto species = names[i];

    if (_root["gas species"].contains(species))
    {
      auto gas_species = _root["gas species"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : gas_species["species"].items())
        this_basis_species[bs.key()] = getReal(bs.value());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::redoxReactions(const std::vector<std::string> & names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    const auto species = names[i];

    if (_root["redox couples"].contains(species))
    {
      auto redox_species = _root["redox couples"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : redox_species["species"].items())
        this_basis_species[bs.key()] = getReal(bs.value());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::oxideReactions(const std::vector<std::string> & names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    const auto species = names[i];

    if (_root["oxides"].contains(species))
    {
      auto oxide_species = _root["oxides"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : oxide_species["species"].items())
        this_basis_species[bs.key()] = getReal(bs.value());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::printReactions(
    const std::vector<std::string> & names,
    const std::vector<std::map<std::string, Real>> & basis_species) const
{
  std::vector<std::string> reactions;

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    std::string reaction = "";
    for (auto & bs : basis_species[i])
    {
      if (bs.second < 0.0)
      {
        if (bs.second == -1.0)
          reaction += " - " + bs.first;
        else
          reaction += " " + Moose::stringify(bs.second) + bs.first;
      }
      else
      {
        if (bs.second == 1.0)
          reaction += " + " + bs.first;
        else
          reaction += " + " + Moose::stringify(bs.second) + bs.first;
      }
    }

    // Trim off leading +
    if (reaction.size() > 1 && reaction[1] == '+')
      reaction.erase(1, 2);

    reactions.push_back(names[i] + " =" + reaction);
  }

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::mineralSpeciesNames() const
{
  std::vector<std::string> names;
  if (_root.contains("mineral species"))
    for (auto & item : _root["mineral species"].items())
      names.push_back(item.key());
  return names;
}

std::vector<std::string>
GeochemicalDatabaseReader::secondarySpeciesNames() const
{
  std::vector<std::string> names;
  if (_root.contains("secondary species"))
    for (auto & item : _root["secondary species"].items())
      names.push_back(item.key());

  if (_root.contains("free electron"))
    for (const auto & nm : _root["free electron"].items())
      names.push_back(nm.key());
  return names;
}

std::vector<std::string>
GeochemicalDatabaseReader::redoxCoupleNames() const
{
  std::vector<std::string> names;
  if (_root.contains("redox couples"))
    for (const auto & item : _root["redox couples"].items())
      names.push_back(item.key());
  return names;
}

std::vector<std::string>
GeochemicalDatabaseReader::surfaceSpeciesNames() const
{
  std::vector<std::string> names;
  if (_root.contains("surface species"))
    for (const auto & item : _root["surface species"].items())
      names.push_back(item.key());
  return names;
}

const FileName &
GeochemicalDatabaseReader::filename() const
{
  return _filename;
}

bool
GeochemicalDatabaseReader::isBasisSpecies(const std::string & name) const
{
  return _root["basis species"].contains(name);
}

bool
GeochemicalDatabaseReader::isRedoxSpecies(const std::string & name) const
{
  return _root["redox couples"].contains(name);
}

bool
GeochemicalDatabaseReader::isSorbingMineral(const std::string & name) const
{
  return _root["sorbing minerals"].contains(name);
}

bool
GeochemicalDatabaseReader::isSecondarySpecies(const std::string & name) const
{
  return _root["secondary species"].contains(name) || _root["free electron"].contains(name);
}

bool
GeochemicalDatabaseReader::isGasSpecies(const std::string & name) const
{
  return _root["gas species"].contains(name);
}

bool
GeochemicalDatabaseReader::isMineralSpecies(const std::string & name) const
{
  return _root["mineral species"].contains(name);
}

bool
GeochemicalDatabaseReader::isOxideSpecies(const std::string & name) const
{
  return _root["oxides"].contains(name);
}

bool
GeochemicalDatabaseReader::isSurfaceSpecies(const std::string & name) const
{
  return _root["surface species"].contains(name);
}

std::string
GeochemicalDatabaseReader::getSpeciesData(const std::string name) const
{
  std::string output;
  for (auto & item : _root.items())
    if (_root[item.key()].contains(name))
    {
      std::ostringstream os;
      os << item.value()[name].dump(4);
      output = os.str();
    }

  if (output.empty())
    mooseError(name + " is not a species in the database");

  return name + ":\n" + output;
}

Real
GeochemicalDatabaseReader::getReal(const nlohmann::json & node)
{
  if (node.is_string())
    return MooseUtils::convert<Real>(node);
  return node;
}
