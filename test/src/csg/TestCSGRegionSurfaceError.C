//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGRegionSurfaceError.h"
#include "MeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGRegionSurfaceError);

InputParameters
TestCSGRegionSurfaceError::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>("input_meshes", "list of MGs");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGRegionSurfaceError::TestCSGRegionSurfaceError(const InputParameters & params)
  : MeshGenerator(params), _mesh_ptrs(getMeshes("input_meshes"))
{
}

std::unique_ptr<MeshBase>
TestCSGRegionSurfaceError::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGRegionSurfaceError::generateCSG()
{
  const auto csg_bases = getCSGBases("input_meshes");
  std::unique_ptr<CSG::CSGBase> csg_1 = std::move(*csg_bases[0]);
  std::unique_ptr<CSG::CSGBase> csg_2 = std::move(*csg_bases[1]);

  // both will have:
  //  cell: square_cell
  //  surfaces: surf_[plus/minus]_[x/y]
  auto cell_1 = csg_1->getCellByName("square_cell");
  auto cell_2 = csg_2->getCellByName("square_cell");

  // try to apply the region from cell_1 to cell_2
  // This will produce an error that the surfaces of the region are not the
  // same surfaces in memory, despite having the same name
  auto & reg_1 = cell_1.getRegion();
  csg_2->updateCellRegion(cell_2, reg_1);

  return csg_1;
}
