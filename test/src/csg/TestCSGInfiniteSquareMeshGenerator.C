//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGInfiniteSquareMeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGInfiniteSquareMeshGenerator);

InputParameters
TestCSGInfiniteSquareMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("side_length", "Side length of infinite square.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGInfiniteSquareMeshGenerator::TestCSGInfiniteSquareMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _side_length(getParam<Real>("side_length"))
{
}

std::unique_ptr<MeshBase>
TestCSGInfiniteSquareMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGInfiniteSquareMeshGenerator::generateCSG()
{
  auto csg_mesh = std::make_unique<CSG::CSGBase>();

  std::string root_univ_name = "root_universe";
  auto root_univ = csg_mesh->createRootUniverse(root_univ_name);

  const auto cell_name = "square_cell";
  const auto material_name = "square_material";
  const auto centroid = Point(0, 0, 0);

  auto elem_cell_ptr = root_univ->addMaterialCell(cell_name, material_name);

  // Add surfaces and halfspaces corresponding to 4 planes of infinite square
  std::vector<std::vector<Point>> points_on_planes {
    {Point( 1. * _side_length / 2., 0., 0.), Point( 1. * _side_length / 2., 1., 0.), Point( 1. * _side_length / 2., 0., 1.)},
    {Point(-1. * _side_length / 2., 0., 0.), Point(-1. * _side_length / 2., 1., 0.), Point(-1. * _side_length / 2., 0., 1.)},
    {Point(0.,  1. * _side_length / 2., 0.), Point(1.,  1. * _side_length / 2., 0.), Point(0.,  1. * _side_length / 2., 1.)},
    {Point(0., -1. * _side_length / 2., 0.), Point(1., -1. * _side_length / 2., 0.), Point(0., -1. * _side_length / 2., 1.)}
  };
  std::vector<std::string> surf_names {"plus_x", "minus_x", "plus_y", "minus_y"};

  for (unsigned int i = 0; i < points_on_planes.size(); ++i)
  {
    const auto surf_name = "surf_" + surf_names[i];
    auto plane_ptr = csg_mesh->createPlaneFromPoints(surf_name, points_on_planes[i][0], points_on_planes[i][1], points_on_planes[i][2]);
    const auto elem_direction = plane_ptr->directionFromPoint(centroid);
    auto elem_halfspace = CSG::CSGHalfspace(plane_ptr, elem_direction);
    elem_cell_ptr->addRegionHalfspace(elem_halfspace);
  }
  return csg_mesh;
}
