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

CSGBase::CSGBase() : _surface_list(CSGSurfaceList()) {}

CSGBase::~CSGBase() {}

void
CSGBase::generateOutput() const
{
  const auto all_surfs = getAllSurfaces();
  for (auto it = all_surfs.begin(); it != all_surfs.end(); it++)
  {
    const auto surf_ptr = it->second;
    const auto surf_name = surf_ptr->getName();
    const auto coeffs = surf_ptr->getCoeffs();
    Moose::out << surf_name << " = openmc.Plane(name='" << surf_name << "', ";
    for (auto it = coeffs.begin(); it != coeffs.end(); ++it)
    {
      if (it != coeffs.begin())
        Moose::out << ", ";
      Moose::out << it->first << "=" << it->second;
    }
    if (surf_ptr->getBoundaryType() != CSG::CSGSurface::BoundaryType::transmission)
      Moose::out << ", boundary_type='vacuum'";
    Moose::out << ")" << std::endl;
  }

  const auto all_cells = _root_universe->getAllCells();
  std::string cell_list = "";
  std::vector<std::string> cell_names;
  for (auto it = all_cells.begin(); it != all_cells.end(); it++)
  {
    const auto cell_ptr = it->second;
    const auto cell_name = cell_ptr->getName();
    cell_names.push_back(cell_name);
    const auto region = cell_ptr->getRegionAsString();
    std::string fill;
    if (cell_ptr->getFillType() == CSG::CSGCell::FillType::material)
    {
      auto mat_cell_ptr = dynamic_cast<CSG::CSGMaterialCell *>(cell_ptr.get());
      fill = mat_cell_ptr->getFill();
    }
    else
      fill = "None";
    Moose::out << cell_name << " = openmc.Cell(name='" << cell_name << "', fill=" << fill
               << ", region=" << region << ")" << std::endl;
  }
  Moose::out << "geometry = openmc.Geometry([" << Moose::stringify(cell_names) << "])" << std::endl;
}
} // namespace CSG
