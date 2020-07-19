//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalDatabaseReader.h"

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
  if (reexpress_free_electron)
    reexpressFreeElectron();
  if (use_piecewise_interpolation && _root["Header"].isMember("logk model"))
    _root["Header"]["logk model"] = "piecewise-linear";
  if (remove_all_extrapolated_secondary_species)
    removeExtrapolatedSecondarySpecies();

  setTemperatures();
  setDebyeHuckel();
  setNeutralSpeciesActivity();
}

void
GeochemicalDatabaseReader::read(FileName filename)
{
  MooseUtils::checkFileReadable(filename);

  // Read the JSON database
  std::ifstream jsondata(filename);
  jsondata >> _root;
}

void
GeochemicalDatabaseReader::reexpressFreeElectron()
{
  if (!_root.isMember("free electron"))
    return;
  if (!_root["free electron"].isMember("e-"))
    return;
  if (!_root["free electron"]["e-"]["species"].isMember("O2(g)"))
    return;
  if (!_root.isMember("basis species"))
    return;
  if (!_root["basis species"].isMember("O2(aq)"))
    return;
  if (!_root.isMember("gas species"))
    return;
  if (!_root["gas species"].isMember("O2(g)"))
    return;
  if (!_root["gas species"]["O2(g)"]["species"].isMember("O2(aq)"))
    return;
  if (_root["gas species"]["O2(g)"]["species"].getMemberNames().size() != 1)
    return;

  // remove O2(g) in the "e-" and replace with O2(aq)
  const std::string stoi_o2g = _root["free electron"]["e-"]["species"]["O2(g)"].asString();
  _root["free electron"]["e-"]["species"].removeMember("O2(g)");
  _root["free electron"]["e-"]["species"]["O2(aq)"] = stoi_o2g;
  const Real stoi = MooseUtils::convert<Real>(stoi_o2g);

  // alter equilibrium constants
  if (!_root["Header"].isMember("temperatures"))
    return;
  for (unsigned i = 0; i < _root["Header"]["temperatures"].size(); ++i)
  {
    const Real logk_e =
        MooseUtils::convert<Real>(_root["free electron"]["e-"]["logk"][i].asString());
    const Real logk_o2 =
        MooseUtils::convert<Real>(_root["gas species"]["O2(g)"]["logk"][i].asString());
    const Real newk = logk_e + stoi * logk_o2;
    _root["free electron"]["e-"]["logk"][i] = std::to_string(newk);
  }
}

void
GeochemicalDatabaseReader::removeExtrapolatedSecondarySpecies()
{
  const std::vector<std::string> names(_root["secondary species"].getMemberNames());
  for (const auto & name : names)
    if (_root["secondary species"][name].isMember("note"))
      _root["secondary species"].removeMember(name);
}

std::string
GeochemicalDatabaseReader::getActivityModel() const
{
  return _root["Header"]["activity model"].asString();
}

std::string
GeochemicalDatabaseReader::getFugacityModel() const
{
  return _root["Header"]["fugacity model"].asString();
}

std::string
GeochemicalDatabaseReader::getLogKModel() const
{
  return _root["Header"]["logk model"].asString();
}

void
GeochemicalDatabaseReader::setTemperatures()
{
  if (_root["Header"].isMember("temperatures"))
  {
    auto temperatures = _root["Header"]["temperatures"];
    _temperature_points.resize(temperatures.size());
    for (unsigned int i = 0; i < temperatures.size(); ++i)
      _temperature_points[i] = MooseUtils::convert<Real>(temperatures[i].asString());
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
  if (_root["Header"].isMember("pressures"))
  {
    auto pressures = _root["Header"]["pressures"];
    _pressure_points.resize(pressures.size());
    for (unsigned int i = 0; i < pressures.size(); ++i)
      _pressure_points[i] = MooseUtils::convert<Real>(pressures[i].asString());
  }

  return _pressure_points;
}

void
GeochemicalDatabaseReader::setDebyeHuckel()
{
  if (getActivityModel() == "debye-huckel")
  {
    if (_root["Header"].isMember("adh"))
    {
      std::vector<Real> adhvals(_root["Header"]["adh"].size());
      for (unsigned int i = 0; i < _root["Header"]["adh"].size(); ++i)
        adhvals[i] = MooseUtils::convert<Real>(_root["Header"]["adh"][i].asString());
      _debye_huckel.adh = adhvals;
    }

    if (_root["Header"].isMember("bdh"))
    {
      std::vector<Real> bdhvals(_root["Header"]["bdh"].size());
      for (unsigned int i = 0; i < _root["Header"]["bdh"].size(); ++i)
        bdhvals[i] = MooseUtils::convert<Real>(_root["Header"]["bdh"][i].asString());
      _debye_huckel.bdh = bdhvals;
    }

    if (_root["Header"].isMember("bdot"))
    {
      std::vector<Real> bdotvals(_root["Header"]["bdot"].size());
      for (unsigned int i = 0; i < _root["Header"]["bdot"].size(); ++i)
        bdotvals[i] = MooseUtils::convert<Real>(_root["Header"]["bdot"][i].asString());
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
  if (_root.isMember("elements"))
  {

    for (auto & el : _root["elements"].getMemberNames())
    {
      _elements[el].name = _root["elements"][el]["name"].asString();
      _elements[el].molecular_weight =
          MooseUtils::convert<Real>(_root["elements"][el]["molecular weight"].asString());
    }
  }

  return _elements;
}

std::map<std::string, GeochemistryBasisSpecies>
GeochemicalDatabaseReader::getBasisSpecies(const std::vector<std::string> & names)
{
  // Parse the basis species specified in names
  for (const auto & species : names)
    if (_root["basis species"].isMember(species))
    {
      GeochemistryBasisSpecies dbs;

      auto basis_species = _root["basis species"][species];
      dbs.name = species;
      dbs.radius = MooseUtils::convert<Real>(basis_species["radius"].asString());
      dbs.charge = MooseUtils::convert<Real>(basis_species["charge"].asString());
      dbs.molecular_weight =
          MooseUtils::convert<Real>(basis_species["molecular weight"].asString());

      std::map<std::string, Real> elements;
      for (auto & el : basis_species["elements"].getMemberNames())
        elements[el] = MooseUtils::convert<Real>(basis_species["elements"][el].asString());

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
    if (_root["secondary species"].isMember(species) or _root["free electron"].isMember(species))
    {
      GeochemistryEquilibriumSpecies dbs;

      auto sec_species = _root["secondary species"].isMember(species)
                             ? _root["secondary species"][species]
                             : _root["free electron"][species];
      dbs.name = species;
      dbs.radius = MooseUtils::convert<Real>(sec_species["radius"].asString());
      dbs.charge = MooseUtils::convert<Real>(sec_species["charge"].asString());
      dbs.molecular_weight = MooseUtils::convert<Real>(sec_species["molecular weight"].asString());

      std::vector<Real> eq_const(sec_species["logk"].size());
      for (unsigned int i = 0; i < sec_species["logk"].size(); ++i)
        eq_const[i] = MooseUtils::convert<Real>(sec_species["logk"][i].asString());

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : sec_species["species"].getMemberNames())
        basis_species[bs] = MooseUtils::convert<Real>(sec_species["species"][bs].asString());

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
    if (_root["mineral species"].isMember(species))
    {
      GeochemistryMineralSpecies dbs;

      auto mineral_species = _root["mineral species"][species];
      dbs.name = species;
      dbs.molecular_weight =
          MooseUtils::convert<Real>(mineral_species["molecular weight"].asString());
      dbs.molecular_volume = MooseUtils::convert<Real>(mineral_species["molar volume"].asString());

      std::vector<Real> eq_const(mineral_species["logk"].size());
      for (unsigned int i = 0; i < mineral_species["logk"].size(); ++i)
        eq_const[i] = MooseUtils::convert<Real>(mineral_species["logk"][i].asString());

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : mineral_species["species"].getMemberNames())
        basis_species[bs] = MooseUtils::convert<Real>(mineral_species["species"][bs].asString());

      dbs.basis_species = basis_species;

      // recover sorption information, if any
      std::map<std::string, Real> species_and_sorbing_density;
      dbs.surface_area = 0.0;
      if (_root["sorbing minerals"].isMember(species))
      {
        auto sorbing_mineral = _root["sorbing minerals"][species];
        dbs.surface_area = MooseUtils::convert<Real>(sorbing_mineral["surface area"].asString());

        for (auto & site : sorbing_mineral["sorbing sites"].getMemberNames())
          species_and_sorbing_density[site] =
              MooseUtils::convert<Real>(sorbing_mineral["sorbing sites"][site].asString());
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
    if (_root["gas species"].isMember(species))
    {
      GeochemistryGasSpecies dbs;

      auto gas_species = _root["gas species"][species];
      dbs.name = species;
      dbs.molecular_weight = MooseUtils::convert<Real>(gas_species["molecular weight"].asString());

      std::vector<Real> eq_const(gas_species["logk"].size());
      for (unsigned int i = 0; i < gas_species["logk"].size(); ++i)
        eq_const[i] = MooseUtils::convert<Real>(gas_species["logk"][i].asString());

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : gas_species["species"].getMemberNames())
        basis_species[bs] = MooseUtils::convert<Real>(gas_species["species"][bs].asString());

      dbs.basis_species = basis_species;

      // Optional fugacity coefficients
      if (gas_species.isMember("chi"))
      {
        std::vector<Real> chi(gas_species["chi"].size());
        for (unsigned int i = 0; i < gas_species["chi"].size(); ++i)
          chi[i] = MooseUtils::convert<Real>(gas_species["chi"][i].asString());

        dbs.chi = chi;
      }

      if (gas_species.isMember("Pcrit"))
        dbs.Pcrit = MooseUtils::convert<Real>(gas_species["Pcrit"].asString());

      if (gas_species.isMember("Tcrit"))
        dbs.Tcrit = MooseUtils::convert<Real>(gas_species["Tcrit"].asString());

      if (gas_species.isMember("omega"))
        dbs.omega = MooseUtils::convert<Real>(gas_species["omega"].asString());

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
    if (_root["redox couples"].isMember(species))
    {
      GeochemistryRedoxSpecies dbs;

      auto redox_species = _root["redox couples"][species];
      dbs.name = species;
      dbs.radius = MooseUtils::convert<Real>(redox_species["radius"].asString());
      dbs.charge = MooseUtils::convert<Real>(redox_species["charge"].asString());
      dbs.molecular_weight =
          MooseUtils::convert<Real>(redox_species["molecular weight"].asString());

      std::vector<Real> eq_const(redox_species["logk"].size());
      for (unsigned int i = 0; i < redox_species["logk"].size(); ++i)
        eq_const[i] = MooseUtils::convert<Real>(redox_species["logk"][i].asString());

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : redox_species["species"].getMemberNames())
        basis_species[bs] = MooseUtils::convert<Real>(redox_species["species"][bs].asString());

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
    if (_root["oxides"].isMember(species))
    {
      GeochemistryOxideSpecies dbs;

      auto oxide_species = _root["oxides"][species];
      dbs.name = species;
      dbs.molecular_weight =
          MooseUtils::convert<Real>(oxide_species["molecular weight"].asString());

      std::map<std::string, Real> basis_species;
      for (auto & bs : oxide_species["species"].getMemberNames())
        basis_species[bs] = MooseUtils::convert<Real>(oxide_species["species"][bs].asString());

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
    if (_root["surface species"].isMember(species))
    {
      GeochemistrySurfaceSpecies dbs;

      auto surface_species = _root["surface species"][species];
      dbs.name = species;
      dbs.charge = MooseUtils::convert<Real>(surface_species["charge"].asString());
      dbs.molecular_weight =
          MooseUtils::convert<Real>(surface_species["molecular weight"].asString());
      dbs.log10K = MooseUtils::convert<Real>(surface_species["log K"].asString());
      dbs.dlog10KdT = MooseUtils::convert<Real>(surface_species["dlogK/dT"].asString());

      std::map<std::string, Real> basis_species;
      for (auto & bs : surface_species["species"].getMemberNames())
        basis_species[bs] = MooseUtils::convert<Real>(surface_species["species"][bs].asString());

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
  if (_root["Header"].isMember("neutral species"))
  {
    auto neutral_species = _root["Header"]["neutral species"];
    for (auto & ns : neutral_species.getMemberNames())
    {
      std::vector<std::vector<Real>> coeffs;

      for (auto & nsac : neutral_species[ns].getMemberNames())
      {
        if (nsac == "note")
          continue;
        std::vector<Real> coeffvec(neutral_species[ns][nsac].size());

        for (unsigned int i = 0; i < coeffvec.size(); ++i)
          coeffvec[i] = MooseUtils::convert<Real>(neutral_species[ns][nsac][i].asString());

        coeffs.push_back(coeffvec);
      }

      // GeochemistryNeutralSpeciesActivity expects four vectos, so
      // add empty vectors if coeffs.size() != 4
      coeffs.resize(4, {});

      GeochemistryNeutralSpeciesActivity nsa(coeffs);

      _neutral_species_activity[ns] = nsa;
    }
  }
}

const std::map<std::string, GeochemistryNeutralSpeciesActivity> &
GeochemicalDatabaseReader::getNeutralSpeciesActivity() const
{
  if (!_root["Header"].isMember("neutral species"))
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

    if (_root["secondary species"].isMember(species))
    {
      auto sec_species = _root["secondary species"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : sec_species["species"].getMemberNames())
        this_basis_species[bs] = MooseUtils::convert<Real>(sec_species["species"][bs].asString());

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

    if (_root["mineral species"].isMember(species))
    {
      auto min_species = _root["mineral species"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : min_species["species"].getMemberNames())
        this_basis_species[bs] = MooseUtils::convert<Real>(min_species["species"][bs].asString());

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

    if (_root["gas species"].isMember(species))
    {
      auto gas_species = _root["gas species"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : gas_species["species"].getMemberNames())
        this_basis_species[bs] = MooseUtils::convert<Real>(gas_species["species"][bs].asString());

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

    if (_root["redox couples"].isMember(species))
    {
      auto redox_species = _root["redox couples"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : redox_species["species"].getMemberNames())
        this_basis_species[bs] = MooseUtils::convert<Real>(redox_species["species"][bs].asString());

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

    if (_root["oxides"].isMember(species))
    {
      auto oxide_species = _root["oxides"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : oxide_species["species"].getMemberNames())
        this_basis_species[bs] = MooseUtils::convert<Real>(oxide_species["species"][bs].asString());

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
    std::vector<std::string> names, std::vector<std::map<std::string, Real>> basis_species) const
{
  // Length of the input vectors must be equal
  if (names.size() != basis_species.size())
    mooseError("Input vectors for printReactions() must be equal size");

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
GeochemicalDatabaseReader::secondarySpeciesNames() const
{
  std::vector<std::string> names(_root["secondary species"].getMemberNames());
  for (const auto nm : _root["free electron"].getMemberNames())
    names.push_back(nm);
  return names;
}

std::vector<std::string>
GeochemicalDatabaseReader::redoxCoupleNames() const
{
  std::vector<std::string> names(_root["redox couples"].getMemberNames());
  return names;
}

std::vector<std::string>
GeochemicalDatabaseReader::surfaceSpeciesNames() const
{
  std::vector<std::string> names(_root["surface species"].getMemberNames());
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
  return _root["basis species"].isMember(name);
}

bool
GeochemicalDatabaseReader::isRedoxSpecies(const std::string & name) const
{
  return _root["redox couples"].isMember(name);
}

bool
GeochemicalDatabaseReader::isSorbingMineral(const std::string & name) const
{
  return _root["sorbing minerals"].isMember(name);
}

bool
GeochemicalDatabaseReader::isSecondarySpecies(const std::string & name) const
{
  return _root["secondary species"].isMember(name) || _root["free electron"].isMember(name);
}

bool
GeochemicalDatabaseReader::isGasSpecies(const std::string & name) const
{
  return _root["gas species"].isMember(name);
}

bool
GeochemicalDatabaseReader::isMineralSpecies(const std::string & name) const
{
  return _root["mineral species"].isMember(name);
}

bool
GeochemicalDatabaseReader::isOxideSpecies(const std::string & name) const
{
  return _root["oxides"].isMember(name);
}

bool
GeochemicalDatabaseReader::isSurfaceSpecies(const std::string & name) const
{
  return _root["surface species"].isMember(name);
}

std::string
GeochemicalDatabaseReader::getSpeciesData(const std::string name) const
{
  moosecontrib::Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "    ";
  builder.settings_["precision"] = 4;
  std::unique_ptr<moosecontrib::Json::StreamWriter> writer(builder.newStreamWriter());

  std::string output;

  for (auto & key : _root.getMemberNames())
    if (_root[key].isMember(name))
    {
      std::ostringstream os;
      writer->write(_root[key][name], &os);
      output = os.str();
    }

  if (output.empty())
    mooseError(name + " is not a species in the database");

  return name + ":\n" + output;
}
