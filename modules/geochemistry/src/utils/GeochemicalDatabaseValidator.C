//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalDatabaseValidator.h"

#include "Conversion.h"
#include "MooseUtils.h"
#include "string"

GeochemicalDatabaseValidator::GeochemicalDatabaseValidator(const FileName filename,
                                                           const moosecontrib::Json::Value & db)
  : _filename(filename), _root(db)
{
}

void
GeochemicalDatabaseValidator::validate()
{
  // Check that the database has a Header key (required)
  if (!_root.isMember("Header"))
    mooseError("The MOOSE database ", _filename, " does not have a required \"Header\" field");

  // Check that temperatures are present
  checkHeaderField("temperatures");

  // The size of the temperature array is used to check all other arrays, which must
  // have the same number of elements
  _temperature_size = _root["Header"]["temperatures"].size();

  // Now check that the values are all Real
  checkHeaderArray("temperatures");

  // Check the pressure values are present and are Reals
  checkHeaderField("pressures");
  checkHeaderArray("pressures");

  // Check activity model coefficients
  checkHeaderField("activity model");

  // If the activity model is Debye-Huckel, test the coefficients are also present
  // and are Reals
  if (_root["Header"]["activity model"] == "debye-huckel")
  {
    checkHeaderField("adh");
    checkHeaderArray("adh");

    checkHeaderField("bdh");
    checkHeaderArray("bdh");

    checkHeaderField("bdot");
    checkHeaderArray("bdot");
  }

  // Check fugacity model is specified
  checkHeaderField("fugacity model");

  // If there are neutral species, check their arrays of coefficients
  if (_root["Header"].isMember("neutral species"))
    for (auto ns : _root["Header"]["neutral species"].getMemberNames())
      for (auto coeffs : _root["Header"]["neutral species"][ns].getMemberNames())
      {
        if (coeffs == "note")
          continue;

        // Check that all temperature values are real numbers
        auto values = _root["Header"]["neutral species"][ns][coeffs];

        checkArraySize(values, "Header:nutral species:" + ns + ":" + coeffs);
        checkArrayValues(values, "Header:nutral species:" + ns + ":" + coeffs);
      }

  // Check the element data
  for (auto & el : _root["elements"].getMemberNames())
    checkElements(el);

  // Check the basis species data
  for (auto & species : _root["basis species"].getMemberNames())
    checkBasisSpecies(species);

  // Check the secondary species data
  if (_root.isMember("secondary species"))
    for (auto & species : _root["secondary species"].getMemberNames())
      checkSecondarySpecies(species);

  // Check the mineral species data
  if (_root.isMember("mineral species"))
    for (auto & species : _root["mineral species"].getMemberNames())
      checkMineralSpecies(species);

  // Check the sorbing mineral species data
  if (_root.isMember("sorbing minerals"))
    for (auto & species : _root["sorbing minerals"].getMemberNames())
      checkSorbingMineralSpecies(species);

  // Check the gas species data
  if (_root.isMember("gas species"))
    for (auto & species : _root["gas species"].getMemberNames())
      checkGasSpecies(species);

  // Check the redox couple data
  if (_root.isMember("redox couples"))
    for (auto & species : _root["redox couples"].getMemberNames())
      checkRedoxSpecies(species);

  // Check the oxide species data
  if (_root.isMember("oxides"))
    for (auto & species : _root["oxides"].getMemberNames())
      checkOxideSpecies(species);

  // Check the surface species data
  if (_root.isMember("surface species"))
    for (auto & species : _root["surface species"].getMemberNames())
      checkSurfaceSpecies(species);
}

bool
GeochemicalDatabaseValidator::isValueReal(const moosecontrib::Json::Value & value) const
{
  try
  {
    MooseUtils::convert<Real>(value.asString(), true);
  }
  catch (const std::invalid_argument & err)
  {
    return false;
  }

  return true;
}

void
GeochemicalDatabaseValidator::checkArrayValues(const moosecontrib::Json::Value & array,
                                               const std::string field) const
{
  // Check each value in the array can be successfully converted to a Real
  for (auto & value : array)
    if (!isValueReal(value))
      mooseError("Array value ",
                 value.asString(),
                 " in the ",
                 field,
                 " field of ",
                 _filename,
                 " cannot be converted to Real");
}

void
GeochemicalDatabaseValidator::checkArrayValues(const std::string type,
                                               const std::string species,
                                               const std::string field) const
{
  // Check each value in the array can be successfully converted to a Real
  for (auto & value : _root[type][species][field])
    if (!isValueReal(value))
      mooseError("Array value ",
                 value.asString(),
                 " in the ",
                 field,
                 " field of ",
                 type,
                 " ",
                 species,
                 " in ",
                 _filename,
                 " cannot be converted to Real");
}

void
GeochemicalDatabaseValidator::checkArraySize(const moosecontrib::Json::Value & array,
                                             const std::string field) const
{
  if (array.size() != _temperature_size)
    mooseError("The number of values in the ",
               field,
               " field of ",
               _filename,
               " is not equal to the number of temperature values");
}

void
GeochemicalDatabaseValidator::checkArraySize(const std::string type,
                                             const std::string species,
                                             const std::string field) const
{
  if (_root[type][species][field].size() != _temperature_size)
    mooseError("The number of values in the ",
               field,
               " field of ",
               type,
               " ",
               species,
               " in ",
               _filename,
               " is not equal to the number of temperature values");
}

void
GeochemicalDatabaseValidator::checkHeaderField(const std::string field) const
{
  if (!_root["Header"].isMember(field))
    mooseError(
        "The MOOSE database ", _filename, " does not have a required \"Header:", field, "\" field");
}

void
GeochemicalDatabaseValidator::checkHeaderArray(const std::string field) const
{
  // Check that all values are real numbers
  auto values = _root["Header"][field];

  checkArraySize(values, "Header:" + field);
  checkArrayValues(values, "Header:" + field);
}

void
GeochemicalDatabaseValidator::checkSpeciesValue(const std::string type,
                                                const std::string species,
                                                const std::string field) const
{
  if (!_root[type][species][field])
    mooseError("The ", type, " ", species, " in ", _filename, " does not have a ", field);

  // The field value should be a real number
  if (!isValueReal(_root[type][species][field]))
    mooseError(field,
               " value ",
               _root[type][species][field].asString(),
               " of the ",
               type,
               " ",
               species,
               " in ",
               _filename,
               " cannot be converted to Real");
}

void
GeochemicalDatabaseValidator::checkSpeciesWeightValue(const std::string type,
                                                      const std::string species,
                                                      const std::string field) const
{
  if (!_root[type][species][field])
    mooseError("The ", type, " ", species, " in ", _filename, " does not have a ", field);

  // Each weight value for each constituent should be a real number
  for (auto & item : _root[type][species][field].getMemberNames())
    if (!isValueReal(_root[type][species][field][item]))
      mooseError("Weight value ",
                 _root[type][species][field][item].asString(),
                 " of constituent ",
                 item,
                 " of the ",
                 type,
                 " ",
                 species,
                 " in ",
                 _filename,
                 " cannot be converted to Real");
}

void
GeochemicalDatabaseValidator::checkElements(const std::string element) const
{
  // Check molecular weight can be converted to a Real
  checkSpeciesValue("elements", element, "molecular weight");
}

void
GeochemicalDatabaseValidator::checkBasisSpecies(const std::string species) const
{
  // Check molecular weight can be converted to a Real
  checkSpeciesValue("basis species", species, "molecular weight");

  // Check charge can be converted to a Real
  checkSpeciesValue("basis species", species, "charge");

  // Check ionic radius can be converted to a Real
  checkSpeciesValue("basis species", species, "radius");

  // Check element weights can be converted to a Real
  checkSpeciesWeightValue("basis species", species, "elements");
}

void
GeochemicalDatabaseValidator::checkSecondarySpecies(const std::string species) const
{
  // Check molecular weight can be converted to a Real
  checkSpeciesValue("secondary species", species, "molecular weight");

  // Check charge can be converted to a Real
  checkSpeciesValue("secondary species", species, "charge");

  // Check ionic radius can be converted to a Real
  checkSpeciesValue("secondary species", species, "radius");

  // Check basis species weights can be converted to a Real
  checkSpeciesWeightValue("secondary species", species, "species");

  // Check the number of logk values and whether they can be converted to a Real
  checkArraySize("secondary species", species, "logk");
  checkArrayValues("secondary species", species, "logk");
}

void
GeochemicalDatabaseValidator::checkMineralSpecies(const std::string species) const
{
  // Check molecular weight can be converted to a Real
  checkSpeciesValue("mineral species", species, "molecular weight");

  // Check molar volume can be converted to a Real
  checkSpeciesValue("mineral species", species, "molar volume");

  // Check basis species weights can be converted to a Real
  checkSpeciesWeightValue("mineral species", species, "species");

  // Check the number of logk values and whether they can be converted to a Real
  checkArraySize("mineral species", species, "logk");
  checkArrayValues("mineral species", species, "logk");
}

void
GeochemicalDatabaseValidator::checkSorbingMineralSpecies(const std::string species) const
{
  // Check surface area can be converted to a Real
  checkSpeciesValue("sorbing minerals", species, "surface area");

  // Check sorbing site weights can be converted to a Real
  checkSpeciesWeightValue("sorbing minerals", species, "sorbing sites");
}

void
GeochemicalDatabaseValidator::checkGasSpecies(const std::string species) const
{
  // Check molecular weight can be converted to a Real
  checkSpeciesValue("gas species", species, "molecular weight");

  // Check basis species weights can be converted to a Real
  checkSpeciesWeightValue("gas species", species, "species");

  // Check the number of logk values and whether they can be converted to a Real
  checkArraySize("gas species", species, "logk");
  checkArrayValues("gas species", species, "logk");

  // Check optional fugacity coefficients
  if (_root["gas species"][species].isMember("chi"))
    checkArrayValues("gas species", species, "chi");

  if (_root["gas species"][species].isMember("Pcrit"))
    checkSpeciesValue("gas species", species, "Pcrit");

  if (_root["gas species"][species].isMember("Tcrit"))
    checkSpeciesValue("gas species", species, "Tcrit");

  if (_root["gas species"][species].isMember("omega"))
    checkSpeciesValue("gas species", species, "omega");
}

void
GeochemicalDatabaseValidator::checkRedoxSpecies(const std::string species) const
{
  // Check molecular weight can be converted to a Real
  checkSpeciesValue("redox couples", species, "molecular weight");

  // Check charge can be converted to a Real
  checkSpeciesValue("redox couples", species, "charge");

  // Check ionic radius can be converted to a Real
  checkSpeciesValue("redox couples", species, "radius");

  // Check basis species weights can be converted to a Real
  checkSpeciesWeightValue("redox couples", species, "species");

  // Check the number of logk values and whether they can be converted to a Real
  checkArraySize("redox couples", species, "logk");
  checkArrayValues("redox couples", species, "logk");
}

void
GeochemicalDatabaseValidator::checkOxideSpecies(const std::string species) const
{
  // Check molecular weight can be converted to a Real
  checkSpeciesValue("oxides", species, "molecular weight");

  // Check basis species weights can be converted to a Real
  checkSpeciesWeightValue("oxides", species, "species");
}

void
GeochemicalDatabaseValidator::checkSurfaceSpecies(const std::string species) const
{
  // Check molecular weight can be converted to a Real
  checkSpeciesValue("surface species", species, "molecular weight");

  // Check charge can be converted to a Real
  checkSpeciesValue("surface species", species, "charge");

  // Check basis species weights can be converted to a Real
  checkSpeciesWeightValue("surface species", species, "species");

  // Check equilibrium constants can be converted to a Real
  checkSpeciesValue("surface species", species, "log K");
  checkSpeciesValue("surface species", species, "dlogK/dT");
}
