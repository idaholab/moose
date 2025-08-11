//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGUniverseFromCellsList.h"
#include "CSGSphere.h"

registerMooseObject("MooseTestApp", TestCSGUniverseFromCellsList);

InputParameters
TestCSGUniverseFromCellsList::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<std::vector<Real>>("radii", "radii of sphere cells.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGUniverseFromCellsList::TestCSGUniverseFromCellsList(const InputParameters & params)
  : MeshGenerator(params), _radii(getParam<std::vector<Real>>("radii"))
{
}

std::unique_ptr<MeshBase>
TestCSGUniverseFromCellsList::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGUniverseFromCellsList::generateCSG()
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto mg_name = this->name();

  // collect a list of cells to add to a new universe later
  std::vector<std::reference_wrapper<const CSG::CSGCell>> cells_to_add;
  // create each cell from the sphere surfaces of the specified radii
  for (unsigned int i = 0; i < _radii.size() - 1; ++i)
  {
    // create a sphere surface of the specified radius at the origin
    std::unique_ptr<CSG::CSGSurface> sp_ptr =
        std::make_unique<CSG::CSGSphere>(mg_name + "_sphere_surf_" + std::to_string(i), _radii[i]);
    auto & sphere_surf = csg_obj->addSurface(sp_ptr);
    // create cell from surface and add it to the list
    const auto & sph_cell =
        csg_obj->createCell(mg_name + "_sphere_cell_" + std::to_string(i), -sphere_surf);
    cells_to_add.push_back(sph_cell);
  }
  // create a new universe that holds all of the cells in the list
  auto & contain_univ = csg_obj->createUniverse(mg_name + "_univ", cells_to_add);

  // need to link universe back to root
  // remove cells from root
  csg_obj->removeCellsFromUniverse(csg_obj->getRootUniverse(), cells_to_add);
  // use the last radius in list to make a new cell to encompass all others
  auto radius = _radii.back();
  std::unique_ptr<CSG::CSGSurface> sp_ptr =
      std::make_unique<CSG::CSGSphere>(mg_name + "_contain_surf", radius);
  auto & sphere_surf = csg_obj->addSurface(sp_ptr);
  csg_obj->createCell(mg_name + "_contain_cell", contain_univ, -sphere_surf);

  return csg_obj;
}
