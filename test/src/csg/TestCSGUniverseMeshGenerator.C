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
  params.addRequiredParam<std::vector<Real>>(
      "input_boxes",
      "each of the side lengths of bounding boxes to contain each of the input mesh universes");
  params.addRequiredParam<std::vector<Real>>(
      "bounding_box", "side lengths (x, y, z) of bounding box for full universe");
  params.addParam<bool>("add_cell_to_univ_mode",
                        false,
                        "use the add/removeCell method when adding the cells to the universe.");
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
    _z_side(getParam<std::vector<Real>>("bounding_box")[2]),
    _add_cell_mode(getParam<bool>("add_cell_to_univ_mode"))
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
  auto mg_name = this->name();
  auto side_lengths = getParam<std::vector<Real>>("input_boxes");

  // start by joining the first two sets of cylinders at the same level and push
  // one level down from root.
  auto csg_bases = getCSGBasesByName(_input_mgs);
  std::unique_ptr<CSG::CSGBase> csg_mesh = std::move(*csg_bases[0]);
  std::string new_base_name = _input_mgs[0] + "_univ";
  std::unique_ptr<CSG::CSGBase> inp_csg_mesh = std::move(*csg_bases[1]);
  std::string new_join_name = _input_mgs[1] + "_univ";

  // joining via this method will move both roots into new universes;
  // subsequent input CSGBases will be joined uisng a different method (below)
  csg_mesh->joinOtherBase(inp_csg_mesh, new_base_name, new_join_name);

  // new universe to collect all others into a main one
  auto new_univ = csg_mesh->createUniverse(mg_name + "_univ");
  // collect a list of cells to add to this new universe
  std::vector<std::shared_ptr<CSG::CSGCell>> cells_to_add;

  // for all input meshes, create a containment cell, but only join CSGBases
  // for ones that were not joined above (i > 1)
  for (unsigned int i = 0; i < _input_mgs.size(); ++i)
  {
    auto img = _input_mgs[i];

    if (i > 1)
    {
      // join this incoming base at the same level as the other bases that already
      // were joined, so incoming root is renamed but the current root remains
      inp_csg_mesh = std::move(*csg_bases[i]);
      new_join_name = img + "_univ";
      csg_mesh->joinOtherBase(inp_csg_mesh, new_join_name);
    }

    std::string current_univ_name;
    if (i == 0)
      current_univ_name = new_base_name;
    else
      current_univ_name = new_join_name;

    // create a cell containing the new (root) univ
    // cell is located at the origin of the original cells - just use one cell to get origin
    auto inp_cells = csg_mesh->getUniverseByName(current_univ_name)->getAllCells();
    auto tmp_cell = inp_cells[0];
    auto tmp_cell_reg = tmp_cell->getRegion();
    auto cell_surfs = tmp_cell_reg.getSurfaces();
    Real x = 0, y = 0, z = 0;
    for (auto cs : cell_surfs)
    {
      // find first non-plane surf to get the origin
      if (cs->getSurfaceType() == CSG::CSGSurface::SurfaceType::PLANE)
        continue;
      else
      {
        auto coeffs = cs->getCoeffs();
        if (coeffs.find("x0") != coeffs.end())
          x = coeffs.at("x0");
        if (coeffs.find("y0") != coeffs.end())
          y = coeffs.at("y0");
        if (coeffs.find("z0") != coeffs.end())
          z = coeffs.at("z0");
        break;
      }
    }

    // bounding box for new cell - located at the origin
    auto x_pos_surf = csg_mesh->createPlaneFromCoefficients(
        img + "_x_pos_surf", 1.0, 0, 0, x + 0.5 * side_lengths[i]);
    auto x_neg_surf = csg_mesh->createPlaneFromCoefficients(
        img + "_x_neg_surf", 1.0, 0, 0, x - 0.5 * side_lengths[i]);
    auto y_pos_surf = csg_mesh->createPlaneFromCoefficients(
        img + "_y_pos_surf", 0, 1.0, 0, y + 0.5 * side_lengths[i]);
    auto y_neg_surf = csg_mesh->createPlaneFromCoefficients(
        img + "_y_neg_surf", 0, 1.0, 0, y - 0.5 * side_lengths[i]);
    auto z_pos_surf = csg_mesh->createPlaneFromCoefficients(
        img + "_z_pos_surf", 0, 0, 1.0, z + 0.5 * side_lengths[i]);
    auto z_neg_surf = csg_mesh->createPlaneFromCoefficients(
        img + "_z_neg_surf", 0, 0, 1.0, z - 0.5 * side_lengths[i]);
    auto new_region =
        -x_pos_surf & +x_neg_surf & -y_pos_surf & +y_neg_surf & -z_pos_surf & +z_neg_surf;

    // create a cell and add it to the new universe
    auto univ_ptr = csg_mesh->getUniverseByName(current_univ_name);
    std::string new_cell_name = img + "_cell";
    if (_add_cell_mode)
    {
      // don't add to the new universe right away, do so later with the addCellsToUniverse method
      auto img_cell = csg_mesh->createCell(new_cell_name, univ_ptr, new_region);
      cells_to_add.push_back(img_cell);
    }
    else // add to the new universe at time of creation
      auto img_cell = csg_mesh->createCell(new_cell_name, univ_ptr, new_region, new_univ);
  }

  if (_add_cell_mode)
  {
    // add the list of cells to the new universe now and remove from root universe
    csg_mesh->addCellsToUniverse(new_univ, cells_to_add);
    csg_mesh->removeCellsFromUniverse(csg_mesh->getRootUniverse(), cells_to_add);
  }

  // make cell with surfaces from bounding_box input and fill cell with new universe containing the
  // other cells
  auto bc_vac = CSG::CSGSurface::BoundaryType::VACUUM; // vacuum bc for bounding box
  auto x_pos_surf = csg_mesh->createPlaneFromCoefficients(
      mg_name + "_bb_x_pos_surf", 1.0, 0, 0, 0.5 * _x_side, bc_vac);
  auto x_neg_surf = csg_mesh->createPlaneFromCoefficients(
      mg_name + "_bb_x_neg_surf", 1.0, 0, 0, -0.5 * _x_side, bc_vac);
  auto y_pos_surf = csg_mesh->createPlaneFromCoefficients(
      mg_name + "_bb_y_pos_surf", 0, 1.0, 0, 0.5 * _y_side, bc_vac);
  auto y_neg_surf = csg_mesh->createPlaneFromCoefficients(
      mg_name + "_bb_y_neg_surf", 0, 1.0, 0, -0.5 * _y_side, bc_vac);
  auto z_pos_surf = csg_mesh->createPlaneFromCoefficients(
      mg_name + "_bb_z_pos_surf", 0, 0, 1.0, 0.5 * _z_side, bc_vac);
  auto z_neg_surf = csg_mesh->createPlaneFromCoefficients(
      mg_name + "_bb_z_neg_surf", 0, 0, 1.0, -0.5 * _z_side, bc_vac);
  auto bb_region =
      -x_pos_surf & +x_neg_surf & -y_pos_surf & +y_neg_surf & -z_pos_surf & +z_neg_surf;

  // create a cell that is added to root
  auto bounding_cell = csg_mesh->createCell(mg_name + "_box", new_univ, bb_region);

  return csg_mesh;
}
