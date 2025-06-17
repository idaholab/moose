//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGUniverseCellModificationError.h"
#include "MeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGUniverseCellModificationError);

InputParameters
TestCSGUniverseCellModificationError::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>("input_meshes", "list of MGs");
  params.addRequiredParam<std::string>("mode", "'add' or 'remove' cell");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGUniverseCellModificationError::TestCSGUniverseCellModificationError(
    const InputParameters & params)
  : MeshGenerator(params),
    _mesh_ptrs(getMeshes("input_meshes")),
    _mode(getParam<std::string>("mode"))
{
}

std::unique_ptr<MeshBase>
TestCSGUniverseCellModificationError::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGUniverseCellModificationError::generateCSG()
{
  // get the two base instances of the input meshes
  // both will have:
  //  cell: square_cell
  //  surfaces: surf_[plus/minus]_[x/y]
  //  one universe: ROOT_UNIVERSE
  const auto csg_bases = getCSGBases("input_meshes");
  std::unique_ptr<CSG::CSGBase> csg_1 = std::move(*csg_bases[0]);
  std::unique_ptr<CSG::CSGBase> csg_2 = std::move(*csg_bases[1]);

  if (_mode == "add")
  {
    // try to add cells from one base instance to a universe in the other
    auto cell_1 = csg_1->getCellByName("square_cell");
    auto new_univ_2 = csg_2->createUniverse("new_univ_2");
    // this will produce an error
    csg_2->addCellToUniverse(new_univ_2, cell_1);
  }
  if (_mode == "remove")
  {
    // try to remove cells defined by one instance from the other
    auto cell_1 = csg_1->getCellByName("square_cell");
    auto root_2 = csg_2->getRootUniverse();
    // this will produce an error
    csg_2->removeCellFromUniverse(root_2, cell_1);
  }

  return csg_1;
}
