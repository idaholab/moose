//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGUniverseCellDeleterGenerator.h"
#include "MeshGenerator.h"
#include "CSGPlane.h"

registerMooseObject("MooseTestApp", TestCSGUniverseCellDeleterGenerator);

InputParameters
TestCSGUniverseCellDeleterGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // input parameter that is an existing mesh generator
  params.addRequiredParam<MeshGeneratorName>("input", "The input MeshGenerator.");
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGUniverseCellDeleterGenerator::TestCSGUniverseCellDeleterGenerator(
    const InputParameters & params)
  : MeshGenerator(params), _mesh_ptr(getMesh("input")), _build_csg(&getCSGBase("input"))
{
}

std::unique_ptr<MeshBase>
TestCSGUniverseCellDeleterGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGUniverseCellDeleterGenerator::generateCSG()
{
  // create a new CSGBase object to join the input with
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // join incoming base as separate universe
  std::string join_name = "tmp_univ";
  csg_obj->joinOtherBase(std::move(*_build_csg), join_name);

  // store data of cell of newly created universe to recreate later
  const auto input_cell_name = getParam<MeshGeneratorName>("input") + "_box_cell";
  const auto & input_cell = csg_obj->getCellByName(input_cell_name);
  const auto input_cell_region = input_cell.getRegion();
  const auto input_cell_fill = input_cell.getFillMaterial();
  const auto & input_cell_transform = input_cell.getTransformations().front();
  const auto cell_transform_type = input_cell_transform.first;
  const auto cell_transform_vals = input_cell_transform.second;

  // delete newly created universe and its cell
  const auto & universe_to_delete = csg_obj->getUniverseByName(join_name);
  const auto cells_to_delete = universe_to_delete.getAllCells();

  csg_obj->deleteUniverse(universe_to_delete);
  for (const auto & cell : cells_to_delete)
    csg_obj->deleteCell(cell.get());

  // Recreate deleted cell and link back to root universe of csg_obj
  const auto & recreated_cell =
      csg_obj->createCell(input_cell_name, input_cell_fill, input_cell_region);
  csg_obj->addTransformation(recreated_cell, cell_transform_type, cell_transform_vals);

  return csg_obj;
}
