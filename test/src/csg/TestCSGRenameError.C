//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGRenameError.h"
#include "MeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGRenameError);

InputParameters
TestCSGRenameError::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>("input_meshes", "list of MGs");
  params.addRequiredParam<std::string>("mode", "universe, cell, or surface error");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGRenameError::TestCSGRenameError(const InputParameters & params)
  : MeshGenerator(params),
    _mesh_ptrs(getMeshes("input_meshes")),
    _mode(getParam<std::string>("mode"))
{
}

std::unique_ptr<MeshBase>
TestCSGRenameError::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGRenameError::generateCSG()
{
  // get the two CSGbase objects of the input meshes
  // both will have:
  //  cell: square_cell
  //  surfaces: surf_[plus/minus]_[x/y]
  //  one universe: ROOT_UNIVERSE
  const auto csg_bases = getCSGBases("input_meshes");
  std::unique_ptr<CSG::CSGBase> csg_1 = std::move(*csg_bases[0]);
  std::unique_ptr<CSG::CSGBase> csg_2 = std::move(*csg_bases[1]);

  if (_mode == "surface")
  {
    // get surface from base 1 of the same name to try to rename in base 2
    const auto & surf_1 = csg_1->getSurfaceByName("surf_plus_x");
    // should produce error that surface is not the same in memory despite having the same name
    csg_2->renameSurface(surf_1, "test_new_surf_name");
  }
  if (_mode == "cell")
  {
    // get cell from base 1 of the same name to try to rename in base 2
    auto & cell_1 = csg_1->getCellByName("square_cell");
    // should produce error that cell is not the same in memory despite having the same name
    csg_2->renameCell(cell_1, "test_new_cell_name");
  }
  if (_mode == "universe")
  {
    // get universe from base 1 of the same name to try to rename in base 2
    auto univ_1 = csg_1->getUniverseByName("ROOT_UNIVERSE");
    // should produce error that universe is not the same in memory despite having the same name
    csg_2->renameUniverse(univ_1, "test_new_univ_name");
  }

  return csg_1;
}
