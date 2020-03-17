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

GeochemicalDatabaseReader::GeochemicalDatabaseReader(const std::string filename)
  : _filename(filename)
{
  read(_filename);
}

void
GeochemicalDatabaseReader::read(FileName filename)
{
  MooseUtils::checkFileReadable(filename);

  // Read the JSON database
  std::ifstream jsondata(filename);
  jsondata >> _root;
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

std::vector<Real>
GeochemicalDatabaseReader::getTemperatures()
{
  // Read temperature points
  if (_root["Header"].isMember("temperatures"))
  {
    auto temperatures = _root["Header"]["temperatures"];
    _temperature_points.resize(temperatures.size());
    for (unsigned int i = 0; i < temperatures.size(); ++i)
      _temperature_points[i] = std::stod(temperatures[i].asString());
  }

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
      _pressure_points[i] = std::stod(pressures[i].asString());
  }

  return _pressure_points;
}

GeochemistryDebyeHuckel
GeochemicalDatabaseReader::getDebyeHuckel()
{
  if (getActivityModel() == "debye-huckel")
  {
    if (_root["Header"].isMember("adh"))
    {
      std::vector<Real> adhvals(_root["Header"]["adh"].size());
      for (unsigned int i = 0; i < _root["Header"]["adh"].size(); ++i)
        adhvals[i] = std::stod(_root["Header"]["adh"][i].asString());
      _debye_huckel.adh = adhvals;
    }

    if (_root["Header"].isMember("bdh"))
    {
      std::vector<Real> bdhvals(_root["Header"]["bdh"].size());
      for (unsigned int i = 0; i < _root["Header"]["bdh"].size(); ++i)
        bdhvals[i] = std::stod(_root["Header"]["bdh"][i].asString());
      _debye_huckel.bdh = bdhvals;
    }

    if (_root["Header"].isMember("bdot"))
    {
      std::vector<Real> bdotvals(_root["Header"]["bdot"].size());
      for (unsigned int i = 0; i < _root["Header"]["bdot"].size(); ++i)
        bdotvals[i] = std::stod(_root["Header"]["bdot"][i].asString());
      _debye_huckel.bdot = bdotvals;
    }
  }
  else
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
          std::stod(_root["elements"][el]["molecular weight"].asString());
    }
  }

  return _elements;
}

std::map<std::string, GeochemistryBasisSpecies>
GeochemicalDatabaseReader::getBasisSpecies(std::vector<std::string> names)
{
  // Parse the basis species specified in names
  for (auto & species : names)
    if (_root["basis species"].isMember(species))
    {
      GeochemistryBasisSpecies dbs;

      auto basis_species = _root["basis species"][species];
      dbs.name = species;
      dbs.radius = std::stod(basis_species["radius"].asString());
      dbs.charge = std::stod(basis_species["charge"].asString());
      dbs.molecular_weight = std::stod(basis_species["molecular weight"].asString());

      std::map<std::string, Real> elements;
      for (auto & el : basis_species["elements"].getMemberNames())
        elements[el] = std::stod(basis_species["elements"][el].asString());

      dbs.elements = elements;

      _basis_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _basis_species;
}

std::map<std::string, GeochemistryEquilibriumSpecies>
GeochemicalDatabaseReader::getEquilibriumSpecies(std::vector<std::string> names)
{
  // Parse the secondary species specified in names
  for (auto & species : names)
    if (_root["secondary species"].isMember(species))
    {
      GeochemistryEquilibriumSpecies dbs;

      auto sec_species = _root["secondary species"][species];
      dbs.name = species;
      dbs.radius = std::stod(sec_species["radius"].asString());
      dbs.charge = std::stod(sec_species["charge"].asString());
      dbs.molecular_weight = std::stod(sec_species["molecular weight"].asString());

      std::vector<Real> eq_const(sec_species["logk"].size());
      for (unsigned int i = 0; i < sec_species["logk"].size(); ++i)
        eq_const[i] = std::stod(sec_species["logk"][i].asString());

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : sec_species["species"].getMemberNames())
        basis_species[bs] = std::stod(sec_species["species"][bs].asString());

      dbs.basis_species = basis_species;

      _equilibrium_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _equilibrium_species;
}

std::map<std::string, GeochemistryMineralSpecies>
GeochemicalDatabaseReader::getMineralSpecies(std::vector<std::string> names)
{
  // Parse the mineral species specified in names
  for (auto & species : names)
    if (_root["mineral species"].isMember(species))
    {
      GeochemistryMineralSpecies dbs;

      auto mineral_species = _root["mineral species"][species];
      dbs.name = species;
      dbs.molecular_weight = std::stod(mineral_species["molecular weight"].asString());
      dbs.molecular_volume = std::stod(mineral_species["molar volume"].asString());

      std::vector<Real> eq_const(mineral_species["logk"].size());
      for (unsigned int i = 0; i < mineral_species["logk"].size(); ++i)
        eq_const[i] = std::stod(mineral_species["logk"][i].asString());

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : mineral_species["species"].getMemberNames())
        basis_species[bs] = std::stod(mineral_species["species"][bs].asString());

      dbs.basis_species = basis_species;

      _mineral_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _mineral_species;
}

std::map<std::string, GeochemistryGasSpecies>
GeochemicalDatabaseReader::getGasSpecies(std::vector<std::string> names)
{
  // Parse the gas species specified in names
  for (auto & species : names)
    if (_root["gas species"].isMember(species))
    {
      GeochemistryGasSpecies dbs;

      auto gas_species = _root["gas species"][species];
      dbs.name = species;
      dbs.molecular_weight = std::stod(gas_species["molecular weight"].asString());

      std::vector<Real> eq_const(gas_species["logk"].size());
      for (unsigned int i = 0; i < gas_species["logk"].size(); ++i)
        eq_const[i] = std::stod(gas_species["logk"][i].asString());

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : gas_species["species"].getMemberNames())
        basis_species[bs] = std::stod(gas_species["species"][bs].asString());

      dbs.basis_species = basis_species;

      // Optional fugacity coefficients
      if (gas_species.isMember("chi"))
      {
        std::vector<Real> chi(gas_species["chi"].size());
        for (unsigned int i = 0; i < gas_species["chi"].size(); ++i)
          chi[i] = std::stod(gas_species["chi"][i].asString());

        dbs.chi = chi;
      }

      if (gas_species.isMember("Pcrit"))
        dbs.Pcrit = std::stod(gas_species["Pcrit"].asString());

      if (gas_species.isMember("Tcrit"))
        dbs.Tcrit = std::stod(gas_species["Tcrit"].asString());

      if (gas_species.isMember("omega"))
        dbs.omega = std::stod(gas_species["omega"].asString());

      _gas_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _gas_species;
}

std::map<std::string, GeochemistryRedoxSpecies>
GeochemicalDatabaseReader::getRedoxSpecies(std::vector<std::string> names)
{
  // Parse the redox species specified in names
  for (auto & species : names)
    if (_root["redox couples"].isMember(species))
    {
      GeochemistryRedoxSpecies dbs;

      auto redox_species = _root["redox couples"][species];
      dbs.name = species;
      dbs.radius = std::stod(redox_species["radius"].asString());
      dbs.charge = std::stod(redox_species["charge"].asString());
      dbs.molecular_weight = std::stod(redox_species["molecular weight"].asString());

      std::vector<Real> eq_const(redox_species["logk"].size());
      for (unsigned int i = 0; i < redox_species["logk"].size(); ++i)
        eq_const[i] = std::stod(redox_species["logk"][i].asString());

      dbs.equilibrium_const = eq_const;

      std::map<std::string, Real> basis_species;
      for (auto & bs : redox_species["species"].getMemberNames())
        basis_species[bs] = std::stod(redox_species["species"][bs].asString());

      dbs.basis_species = basis_species;

      _redox_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _redox_species;
}

std::map<std::string, GeochemistryOxideSpecies>
GeochemicalDatabaseReader::getOxideSpecies(std::vector<std::string> names)
{
  // Parse the oxide species specified in names
  for (auto & species : names)
    if (_root["oxides"].isMember(species))
    {
      GeochemistryOxideSpecies dbs;

      auto oxide_species = _root["oxides"][species];
      dbs.name = species;
      dbs.molecular_weight = std::stod(oxide_species["molecular weight"].asString());

      std::map<std::string, Real> basis_species;
      for (auto & bs : oxide_species["species"].getMemberNames())
        basis_species[bs] = std::stod(oxide_species["species"][bs].asString());

      dbs.basis_species = basis_species;

      _oxide_species[species] = dbs;
    }
    else
      mooseError(species + " does not exist in database " + _filename);

  return _oxide_species;
}

std::map<std::string, GeochemistryNeutralSpeciesActivity>
GeochemicalDatabaseReader::getNeutralSpeciesActivity()
{
  if (_root["Header"].isMember("neutral species"))
  {
    auto neutral_species = _root["Header"]["neutral species"];
    for (auto & ns : neutral_species.getMemberNames())
    {
      std::vector<std::vector<Real>> coeffs;

      for (auto & nsac : neutral_species[ns].getMemberNames())
      {
        std::vector<Real> coeffvec(neutral_species[ns][nsac].size());

        for (unsigned int i = 0; i < coeffvec.size(); ++i)
          coeffvec[i] = std::stod(neutral_species[ns][nsac][i].asString());

        coeffs.push_back(coeffvec);
      }

      // GeochemistryNeutralSpeciesActivity expects four vectos, so
      // add empty vectors if coeffs.size() != 4
      coeffs.resize(4, {});

      GeochemistryNeutralSpeciesActivity nsa(coeffs);

      _neutral_species_activity[ns] = nsa;
    }
  }
  else
    mooseError("No neutral species activity coefficients in database");

  return _neutral_species_activity;
}

std::vector<std::string>
GeochemicalDatabaseReader::equilibriumReactions(std::vector<std::string> names) const
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
        this_basis_species[bs] = std::stod(sec_species["species"][bs].asString());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::mineralReactions(std::vector<std::string> names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    auto species = names[i];

    if (_root["mineral species"].isMember(species))
    {
      auto min_species = _root["mineral species"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : min_species["species"].getMemberNames())
        this_basis_species[bs] = std::stod(min_species["species"][bs].asString());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::gasReactions(std::vector<std::string> names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    auto species = names[i];

    if (_root["gas species"].isMember(species))
    {
      auto gas_species = _root["gas species"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : gas_species["species"].getMemberNames())
        this_basis_species[bs] = std::stod(gas_species["species"][bs].asString());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::redoxReactions(std::vector<std::string> names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    auto species = names[i];

    if (_root["redox couples"].isMember(species))
    {
      auto redox_species = _root["redox couples"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : redox_species["species"].getMemberNames())
        this_basis_species[bs] = std::stod(redox_species["species"][bs].asString());

      basis_species[i] = this_basis_species;
    }
    else
      mooseError(species + " does not exist in database " + _filename);
  }

  auto reactions = printReactions(names, basis_species);

  return reactions;
}

std::vector<std::string>
GeochemicalDatabaseReader::oxideReactions(std::vector<std::string> names) const
{
  std::vector<std::map<std::string, Real>> basis_species(names.size());

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    auto species = names[i];

    if (_root["oxides"].isMember(species))
    {
      auto oxide_species = _root["oxides"][species];

      // The basis species in this reaction
      std::map<std::string, Real> this_basis_species;
      for (auto & bs : oxide_species["species"].getMemberNames())
        this_basis_species[bs] = std::stod(oxide_species["species"][bs].asString());

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
    std::string reaction;
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
    if (reaction[1] == '+')
      reaction.erase(1, 2);

    reactions.push_back(names[i] + " =" + reaction);
  }

  return reactions;
}
