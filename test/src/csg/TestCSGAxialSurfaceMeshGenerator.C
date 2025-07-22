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

  params.addRequiredParam<MeshGeneratorName>("input", "The input MeshGenerator.");
  params.addRequiredParam<Real>("axial_height", "Axial height of the model.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGAxialSurfaceMeshGenerator::TestCSGAxialSurfaceMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _mesh_ptr(getMesh("input")),
    _axial_height(getParam<Real>("axial_height"))
{
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
  std::unique_ptr<CSG::CSGBase> csg_obj = std::move(getCSGBase("input"));

  auto root_univ = csg_obj->getRootUniverse();
  const auto cell_name = "square_cell";
  auto & csg_cell = root_univ.getCell(cell_name);
  auto cell_region = csg_cell.getRegion();
  const auto centroid = Point(0, 0, 0);

  // Add surfaces and halfspaces corresponding to top and bottom axial planes
  std::vector<std::string> surf_names{"plus_z", "minus_z"};
  std::vector<Real> coeffs{0.5 * _axial_height, -0.5 * _axial_height};
  for (unsigned int i = 0; i < coeffs.size(); ++i)
  {
    const auto surf_name = "surf_" + surf_names[i];
    // z plane equation: 0.0*x + 0.0*y + 1.0*z = (+/-)0.5 * axial_height
    std::unique_ptr<CSG::CSGSurface> surface_ptr =
        std::make_unique<CSG::CSGPlane>(surf_name, 0.0, 0.0, 1.0, coeffs[i]);
    auto & csg_plane = csg_obj->addSurface(surface_ptr);
    const auto region_direction = csg_plane.getHalfspaceFromPoint(centroid);
    auto halfspace =
        ((region_direction == CSG::CSGSurface::Halfspace::POSITIVE) ? +csg_plane : -csg_plane);
    cell_region &= halfspace;
  }

  csg_obj->updateCellRegion(csg_cell, cell_region);

  return csg_obj;
}
