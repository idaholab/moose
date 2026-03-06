//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGAxialSurfaceMeshGenerator.h"
#include "MeshGenerator.h"
#include "CSGPlane.h"

registerMooseObject("MooseTestApp", TestCSGAxialSurfaceMeshGenerator);

InputParameters
TestCSGAxialSurfaceMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // input parameter that is an existing mesh generator
  params.addRequiredParam<MeshGeneratorName>("input", "The input MeshGenerator.");
  // additional params for this specific mesh generator
  params.addRequiredParam<Real>("axial_height", "Axial height of the model.");
  params.addParam<Real>("z_rotation",
                        "optional rotation around the z axis to apply to the generated cell.");
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGAxialSurfaceMeshGenerator::TestCSGAxialSurfaceMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _mesh_ptr(getMesh("input")),
    _axial_height(getParam<Real>("axial_height")),
    _rotation(isParamValid("z_rotation") ? getParam<Real>("z_rotation") : 0.0)
{
  _build_csg = &getCSGBase("input");
}

std::unique_ptr<MeshBase>
TestCSGAxialSurfaceMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGAxialSurfaceMeshGenerator::generateCSG()
{
  // get the existing CSGBase associated with the input mesh generator
  // this is the CSGBase object that will be updated
  std::unique_ptr<CSG::CSGBase> csg_obj = std::move(*_build_csg);

  // get the names of the current mesh generator and the input mesh generator
  // so that unique object naming can be enforced
  auto mg_name = this->name();
  auto inp_name = getParam<MeshGeneratorName>("input");

  // get the expected existing cell
  const auto cell_name = inp_name + "_square_cell";
  const auto & csg_cell = csg_obj->getCellByName(cell_name);

  // get the existing cell region to update
  auto cell_region = csg_cell.getRegion();

  // centroid used to determine direction for half-space
  const auto centroid = Point(0, 0, 0);

  // setting a default surface name purely for testing purposes
  const auto default_surf_name = "default_surf";

  // Add surfaces and halfspaces corresponding to top and bottom axial planes
  std::vector<std::string> surf_names{"plus_z", "minus_z"};
  std::vector<Real> coeffs{0.5 * _axial_height, -0.5 * _axial_height};
  for (unsigned int i = 0; i < coeffs.size(); ++i)
  {
    // create a plane using the coefficients for the equation of a plane
    // z plane equation: 0.0*x + 0.0*y + 1.0*z = (+/-)0.5 * axial_height
    std::unique_ptr<CSG::CSGSurface> surface_ptr =
        std::make_unique<CSG::CSGPlane>(default_surf_name, 0.0, 0.0, 1.0, coeffs[i]);
    auto & csg_plane = csg_obj->addSurface(std::move(surface_ptr));

    // Rename surface so that it has a unique surface name based on the mesh generator
    const auto surf_name = mg_name + "_surf_" + surf_names[i];
    csg_obj->renameSurface(csg_plane, surf_name);

    // determine the half-space to add as an updated intersection
    const auto region_direction = csg_plane.getHalfspaceFromPoint(centroid);
    auto halfspace =
        ((region_direction == CSG::CSGSurface::Halfspace::POSITIVE) ? +csg_plane : -csg_plane);

    // update the existing region with a half-space
    cell_region &= halfspace;
  }

  // set the new region for the existing cell
  csg_obj->updateCellRegion(csg_cell, cell_region);

  // Rename cell as it now defines a box region instead of an infinite square region
  csg_obj->renameCell(csg_cell, mg_name + "_box_cell");

  // apply rotation around z-axis if specified
  if (_rotation != 0.0)
    csg_obj->applyAxisRotation(csg_cell, "z", _rotation);

  return csg_obj;
}
