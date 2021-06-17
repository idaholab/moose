//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalModelDefinition.h"
#include "GeochemistryKineticRate.h"

registerMooseObject("GeochemistryApp", GeochemicalModelDefinition);

InputParameters
GeochemicalModelDefinition::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<FileName>("database_file", "The name of the geochemical database file");
  params.addParam<bool>(
      "reexpress_free_electron",
      true,
      "If true then if: (1) the 'free electron' appears in the database file; and (2) its "
      "equilibrium reaction includes O2(g); and (3) O2(g) is a gas; and (4) O2(g)'s equilibrium "
      "reaction is O2(g)=O2(eq); and (5) O2(aq) exists as a basis species in the database file; "
      "then reexpress the free electron's equilibrium reaction in terms of O2(aq).  Note that if "
      "you choose 'reexpress_free_electron=false' and these other 5 conditions are true, then the "
      "'free electron' will not be available as a secondary species");
  params.addParam<bool>(
      "piecewise_linear_interpolation",
      false,
      "If true then use a piecewise-linear interpolation of logK and Debye-Huckel parameters, "
      "regardless of the interpolation type specified in the database file.  This can be useful "
      "for comparing with results using other geochemistry software");
  params.addRequiredParam<std::vector<std::string>>(
      "basis_species",
      "A list of basis components relevant to the aqueous-equilibrium problem. H2O must appear "
      "first in this list.  These components must be chosen from the 'basis species' in the "
      "database, the sorbing sites (if any) and the decoupled redox states that are in "
      "disequilibrium (if any).");
  params.addParam<std::vector<std::string>>(
      "equilibrium_minerals",
      "A list of minerals that are in equilibrium with the aqueous solution.  All members of this "
      "list must be in the 'minerals' section of the database file");
  params.addParam<std::vector<std::string>>(
      "equilibrium_gases",
      "A list of gases that are in equilibrium with the aqueous solution and can have their "
      "fugacities fixed, at least for some time and spatial location.  All members of this list "
      "must be in the 'gas' section of the database file");
  params.addParam<std::vector<std::string>>(
      "kinetic_minerals",
      "A list of minerals whose dynamics are governed by a rate law.  These are not in equilibrium "
      "with the aqueous solution.  All members of this list must be in the 'minerals' section of "
      "the database "
      "file.");
  params.addParam<std::vector<std::string>>(
      "kinetic_redox",
      "A list alternative oxidation states (eg Fe+++) whose dynamics are governed by a rate law.  "
      "These are not in equilibrium with the aqueous solution.  All members of this list must be "
      "in the "
      "'redox couples' section of the database file.");
  params.addParam<std::vector<std::string>>(
      "kinetic_surface_species",
      "A list surface sorbing species whose dynamics are governed by a rate law.  These are not in "
      "equilibrium with the aqueous solution.  All members of this list must be in the 'surface "
      "species' section of the database file.");
  params.addParam<std::string>(
      "redox_oxygen",
      "O2(aq)",
      "The name of the oxygen species that appears in redox reactions.  For redox pairs that are "
      "in disequilibrium to be correctly recorded, and hence their Nernst potentials to be "
      "computed easily, redox_oxygen must be a basis species and it must appear in the reaction "
      "for each redox pair");
  params.addParam<std::string>(
      "redox_electron",
      "e-",
      "The name of the free electron.  For redox pairs that are in disequilibrium to be correctly "
      "recorded, and hence their Nernst potentials to be computed eqsily, the equilibrium reaction "
      "for redox_electron must involve redox_oxygen, and the basis species must be chosen to that "
      "redox_electron is an equilibrium species");
  params.addParam<std::vector<UserObjectName>>(
      "kinetic_rate_descriptions",
      "A list of GeochemistryKineticRate UserObject names that define the kinetic rates.  If a "
      "kinetic species has no rate prescribed to it, its reaction rate will be zero");
  params.addParam<bool>(
      "remove_all_extrapolated_secondary_species",
      false,
      "After reading the database file, immediately remove all secondary species that have "
      "extrapolated equilibrium constants.  Sometimes these extrapolations are completely crazy "
      "and those secondary species greatly impact the results");

  params.addClassDescription("User object that parses a geochemical database file, and only "
                             "retains information relevant to the current geochemical model");

  return params;
}

GeochemicalModelDefinition::GeochemicalModelDefinition(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _db(getParam<FileName>("database_file"),
        getParam<bool>("reexpress_free_electron"),
        getParam<bool>("piecewise_linear_interpolation"),
        getParam<bool>("remove_all_extrapolated_secondary_species")),
    _model(_db,
           getParam<std::vector<std::string>>("basis_species"),
           getParam<std::vector<std::string>>("equilibrium_minerals"),
           getParam<std::vector<std::string>>("equilibrium_gases"),
           getParam<std::vector<std::string>>("kinetic_minerals"),
           getParam<std::vector<std::string>>("kinetic_redox"),
           getParam<std::vector<std::string>>("kinetic_surface_species"),
           getParam<std::string>("redox_oxygen"),
           getParam<std::string>("redox_electron"))
{
  for (const auto & kr : getParam<std::vector<UserObjectName>>("kinetic_rate_descriptions"))
    _model.addKineticRate(getUserObjectByName<GeochemistryKineticRate>(kr).getRateDescription());
}

void
GeochemicalModelDefinition::initialize()
{
}

void
GeochemicalModelDefinition::execute()
{
}

void
GeochemicalModelDefinition::finalize()
{
}

const ModelGeochemicalDatabase &
GeochemicalModelDefinition::getDatabase() const
{
  return _model.modelGeochemicalDatabase();
}

const PertinentGeochemicalSystem &
GeochemicalModelDefinition::getPertinentGeochemicalSystem() const
{
  return _model;
}

const GeochemicalDatabaseReader &
GeochemicalModelDefinition::getOriginalFullDatabase() const
{
  return _db;
}
