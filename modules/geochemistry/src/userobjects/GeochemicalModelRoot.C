//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalModelRoot.h"

registerMooseObject("GeochemistryApp", GeochemicalModelRoot);

defineLegacyParams(GeochemicalModelRoot);

InputParameters
GeochemicalModelRoot::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<FileName>("database_file", "The name of the geochemical database file");
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

  params.addClassDescription("User object that parses a geochemical database file, and only "
                             "retains information relevant to the current geochemical model");

  return params;
}

GeochemicalModelRoot::GeochemicalModelRoot(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _model(GeochemicalDatabaseReader(getParam<FileName>("database_file")),
           getParam<std::vector<std::string>>("basis_species"),
           getParam<std::vector<std::string>>("equilibrium_minerals"),
           getParam<std::vector<std::string>>("equilibrium_gases"),
           getParam<std::vector<std::string>>("kinetic_minerals"),
           getParam<std::vector<std::string>>("kinetic_redox"),
           getParam<std::vector<std::string>>("kinetic_surface_species"))
{
}

void
GeochemicalModelRoot::initialize()
{
}

void
GeochemicalModelRoot::execute()
{
}

void
GeochemicalModelRoot::finalize()
{
}

ModelGeochemicalDatabase
GeochemicalModelRoot::getDatabase() const
{
  return _model.modelGeochemicalDatabaseCopy();
}
