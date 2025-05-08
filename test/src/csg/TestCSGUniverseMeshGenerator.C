//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGUniverseMeshGenerator.h"
#include "MeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGUniverseMeshGenerator);

InputParameters
TestCSGUniverseMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>("input_meshes",
                                                          "list of MGs to add to universe");
  params.addRequiredParam<std::vector<Real>>("bounding_box",
                                             "side lengths (x, y, z) of bounding box");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGUniverseMeshGenerator::TestCSGUniverseMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _mesh_ptrs(getMeshes("input_meshes")),
    _input_mgs(getParam<std::vector<MeshGeneratorName>>("input_meshes")),
    _x_side(getParam<std::vector<Real>>("bounding_box")[0]),
    _y_side(getParam<std::vector<Real>>("bounding_box")[1]),
    _z_side(getParam<std::vector<Real>>("bounding_box")[2])
{
}

std::unique_ptr<MeshBase>
TestCSGUniverseMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGUniverseMeshGenerator::generateCSG()
{
  auto csg_mesh = std::make_unique<CSG::CSGBase>();
  auto new_univ = csg_mesh->createUniverse(this->name() + "_univ");
  auto mg_name = this->name();

  for (auto img : _input_mgs)
  {
    std::unique_ptr<CSG::CSGBase> inp_csg_mesh = std::move(getCSGMeshByName(img));
    // get list of cells that are to be added to the new universe
    auto cells_to_add = inp_csg_mesh->getAllCells();
    csg_mesh->joinOtherBase(inp_csg_mesh); // should join root universes
    for (auto c_pair : cells_to_add)
    {
      auto cname = c_pair.first;
      auto cell_ptr = csg_mesh->getCellByName(cname);

      new_univ->addCell(cell_ptr);
    }
  }

  // make cell with surfaces from bounding_box input and fill cell with new universe containing the
  // other cells
  auto x_pos_surf =
      csg_mesh->createPlaneFromCoefficients(mg_name + "_bb_x_pos_surf", 1.0, 0, 0, 0.5 * _x_side);
  auto x_neg_surf =
      csg_mesh->createPlaneFromCoefficients(mg_name + "_bb_x_neg_surf", 1.0, 0, 0, -0.5 * _x_side);
  auto y_pos_surf =
      csg_mesh->createPlaneFromCoefficients(mg_name + "_bb_y_pos_surf", 0, 1.0, 0, 0.5 * _x_side);
  auto y_neg_surf =
      csg_mesh->createPlaneFromCoefficients(mg_name + "_bb_y_neg_surf", 0, 1.0, 0, -0.5 * _x_side);
  auto z_pos_surf =
      csg_mesh->createPlaneFromCoefficients(mg_name + "_bb_z_pos_surf", 0, 1.0, 0, 0.5 * _x_side);
  auto z_neg_surf =
      csg_mesh->createPlaneFromCoefficients(mg_name + "_bb_z_neg_surf", 0, 1.0, 0, -0.5 * _x_side);
  auto bb_region =
      -x_pos_surf & +x_neg_surf & -y_pos_surf & +y_neg_surf & -z_pos_surf & +z_neg_surf;

  const CSG::CSGUniverse & const_univ = *new_univ;
  auto bounding_cell = csg_mesh->createCell(mg_name + "_box", const_univ, bb_region);

  return csg_mesh;
}
