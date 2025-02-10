//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGBase.h"
#include "CSGMaterialCell.h"

namespace CSG
{

std::shared_ptr<CSGUniverse>
CSGBase::createRootUniverse(const std::string name)
{
  if (_root_universe)
    mooseError("Root universe for this mesh has already been created.");
  _root_universe = std::make_shared<CSGUniverse>(name);
  return _root_universe;
}

std::shared_ptr<CSGUniverse>
CSGBase::getRootUniverse()
{
  if (!_root_universe)
    mooseError("Cannot retrieve root universe before it is initialized.");
  return _root_universe;
}

CSGBase::CSGBase() : _surface_list(CSGSurfaceList()) {}

CSGBase::~CSGBase() {}

void
CSGBase::generateOutput() const
{
  // TODO Kalin logic for generating output JSON file will be added here
}
} // namespace CSG
