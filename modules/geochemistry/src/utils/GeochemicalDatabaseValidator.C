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
                                                           const nlohmann::json & db)
  : _filename(filename), _root(db)
{
}

void
GeochemicalDatabaseValidator::validate()
{
  // Check that the database has a Header key (required)
  if (!_root.contains("Header"))
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
  if (_root["Header"].contains("neutral species"))
    for (auto & ns : _root["Header"]["neutral species"].items())
      for (auto & coeffs : ns.value().items())
      {
        if (coeffs.key() == "note")
          continue;

        // Check that there are a correct number of values and that they are real numbers
        auto values = coeffs.value();
        checkArraySize(values, "Header:neutral species:" + ns.key() + ":" + coeffs.key());
        checkArrayValues(values, "Header:neutral species:" + ns.key() + ":" + coeffs.key());
      }

  // Check the element data
  for (auto & el : _root["elements"].items())
    checkElements(el.key());

  // Check the basis species data
  for (auto & species : _root["basis species"].items())
    checkBasisSpecies(species.key());

  // Check the secondary species data
  if (_root.contains("secondary species"))
    for (auto & species : _root["secondary species"].items())
      checkSecondarySpecies(species.key());

  // Check the mineral species data
  if (_root.contains("mineral species"))
    for (auto & species : _root["mineral species"].items())
      checkMineralSpecies(species.key());

  // Check the sorbing mineral species data
  if (_root.contains("sorbing minerals"))
    for (auto & species : _root["sorbing minerals"].items())
      checkSorbingMineralSpecies(species.key());

  // Check the gas species data
  if (_root.contains("gas species"))
    for (auto & species : _root["gas species"].items())
      checkGasSpecies(species.key());

  // Check the redox couple data
  if (_root.contains("redox couples"))
    for (auto & species : _root["redox couples"].items())
      checkRedoxSpecies(species.key());

  // Check the oxide species data
  if (_root.contains("oxides"))
    for (auto & species : _root["oxides"].items())
      checkOxideSpecies(species.key());

  // Check the surface species data
  if (_root.contains("surface species"))
    for (auto & species : _root["surface species"].items())
      checkSurfaceSpecies(species.key());
}

bool
GeochemicalDatabaseValidator::isValueReal(const nlohmann::json & value) const
{
  if (value.is_number())
    return true;
  try
  {
    MooseUtils::convert<Real>(value, true);
  }
  catch (const std::invalid_argument & err)
  {
    return false;
  }

  return true;
}

void
GeochemicalDatabaseValidator::checkArrayValues(const nlohmann::json & array,
                                               const std::string field) const
{
  // Check each value in the array can be successfully converted to a Real
  for (auto & item : array.items())
    if (!isValueReal(item.value()))
      mooseError("Array value ",
                 nlohmann::to_string(item.value()),
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
  for (auto & item : _root[type][species][field].items())
    if (!isValueReal(item.value()))
      mooseError("Array value ",
                 nlohmann::to_string(item.value()),
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
GeochemicalDatabaseValidator::checkArraySize(const nlohmann::json & array,
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
  if (!_root["Header"].contains(field))
    mooseError(
        "The MOOSE database ", _filename, " does not have a required \"Header:", field, "\" field");
}

void
GeochemicalDatabaseValidator::checkHeaderArray(const std::string field) const
{
  // Check that all values are real numbers and of size equal to the temperature
  checkArraySize(_root["Header"][field], "Header:" + field);
  checkArrayValues(_root["Header"][field], "Header:" + field);
}

void
GeochemicalDatabaseValidator::checkSpeciesValue(const std::string type,
                                                const std::string species,
                                                const std::string field) const
{
  if (!_root[type].contains(species) || !_root[type][species].contains(field))
    mooseError("The ", type, " ", species, " in ", _filename, " does not have a ", field);

  // The field value should be a real number
  if (!isValueReal(_root[type][species][field]))
    mooseError(field,
               " value ",
               nlohmann::to_string(_root[type][species][field]),
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
  if (!_root[type].contains(species) || !_root[type][species].contains(field))
    mooseError("The ", type, " ", species, " in ", _filename, " does not have a ", field);

  // Each weight value for each constituent should be a real number
  for (auto & item : _root[type][species][field].items())
    if (!isValueReal(item.value()))
      mooseError("Weight value ",
                 nlohmann::to_string(item.value()),
                 " of constituent ",
                 item.key(),
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
  if (_root["gas species"][species].contains("chi"))
    checkArrayValues("gas species", species, "chi");

  if (_root["gas species"][species].contains("Pcrit"))
    checkSpeciesValue("gas species", species, "Pcrit");

  if (_root["gas species"][species].contains("Tcrit"))
    checkSpeciesValue("gas species", species, "Tcrit");

  if (_root["gas species"][species].contains("omega"))
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
