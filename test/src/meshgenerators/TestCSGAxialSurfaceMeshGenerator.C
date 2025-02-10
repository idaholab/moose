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

registerMooseObject("MooseTestApp", TestCSGAxialSurfaceMeshGenerator);

InputParameters
TestCSGAxialSurfaceMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The input MeshGenerator.");
  params.addRequiredParam<Real>("axial_height", "Axial height of output CSG mesh.");
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
  std::unique_ptr<CSG::CSGBase> input_mesh = std::move(getCSGMesh("input"));

  auto root_univ = input_mesh->getRootUniverse();
  const auto cell_name = "square_cell";
  auto elem_cell_ptr = root_univ->getCell(cell_name);
  const auto centroid = Point(0, 0, 0);

  // Add surfaces and halfspaces corresponding to top and bottom axial planes
  std::vector<std::vector<Point>> points_on_planes {
    {Point(0., 0.,  1. * _axial_height / 2.), Point(1., 0.,  1. * _axial_height / 2.), Point(0., 1.,  1. * _axial_height / 2.)},
    {Point(0., 0., -1. * _axial_height / 2.), Point(1., 0., -1. * _axial_height / 2.), Point(0., 1., -1. * _axial_height / 2.)}
  };
  std::vector<std::string> surf_names {"plus_z", "minus_z"};

  for (unsigned int i = 0; i < points_on_planes.size(); ++i)
  {
    const auto surf_name = "surf_" + surf_names[i];
    auto plane_ptr = input_mesh->createPlaneFromPoints(surf_name, points_on_planes[i][0], points_on_planes[i][1], points_on_planes[i][2]);
    const auto elem_direction = plane_ptr->directionFromPoint(centroid);
    auto elem_halfspace = CSG::CSGHalfspace(plane_ptr, elem_direction);
    elem_cell_ptr->addRegionHalfspace(elem_halfspace);
  }

  return input_mesh;
}
