//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGBase.h"
#include "MooseApp.h"

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
    nlohmann::json csg_json;

    auto all_cells = _root_universe->getAllCells();

    csg_json["SURFACES"] = {};
    csg_json["CELLS"] = {};
    csg_json["UNIVERSES"] = {};

  // get all surfaces information
  auto all_surfs = getAllSurfaces();
  for (const auto & s : all_surfs)
  {
    // print surface name
    Moose::out << s.first << std::endl;
    auto surf_obj = s.second;
    auto coeffs = surf_obj->getCoeffs();
    csg_json["SURFACES"][s.first] = {{"TYPE", surf_obj->getSurfaceType() }, // not sure how to convert type to str
        {"COEFFICIENTS", {}}
    };

    for (const auto & c : coeffs)
    {
        // print coefficients
        csg_json["SURFACES"][s.first]["COEFFICIENTS"][c.first] = c.second;
        Moose::out << '\t' << c.first << '\t' << c.second
                                                 << std::endl;
    }
  }

  // write generated json to file
  std::string json_out = _root_universe->getName() +  "_csg.json";  // this needs to be the inp file name but I cannot figure out how to gather that
  std::ofstream csg_file;
  csg_file.open(json_out);
  csg_file << csg_json.dump(2);
  csg_file.close();
}
} // namespace CSG
