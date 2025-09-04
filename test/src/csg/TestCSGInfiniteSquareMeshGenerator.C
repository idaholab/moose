//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGInfiniteSquareMeshGenerator.h"
#include "CSGBase.h"
#include "CSGPlane.h"

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

TestCSGInfiniteSquareMeshGenerator::TestCSGInfiniteSquareMeshGenerator(
    const InputParameters & params)
  : MeshGenerator(params), _side_length(getParam<Real>("side_length"))
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
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  const auto centroid = Point(0, 0, 0);

  // Add surfaces and halfspaces corresponding to 4 planes of infinite square
  std::vector<std::vector<Point>> points_on_planes{{Point(1. * _side_length / 2., 0., 0.),
                                                    Point(1. * _side_length / 2., 1., 0.),
                                                    Point(1. * _side_length / 2., 0., 1.)},
                                                   {Point(-1. * _side_length / 2., 0., 0.),
                                                    Point(-1. * _side_length / 2., 1., 0.),
                                                    Point(-1. * _side_length / 2., 0., 1.)},
                                                   {Point(0., 1. * _side_length / 2., 0.),
                                                    Point(1., 1. * _side_length / 2., 0.),
                                                    Point(0., 1. * _side_length / 2., 1.)},
                                                   {Point(0., -1. * _side_length / 2., 0.),
                                                    Point(1., -1. * _side_length / 2., 0.),
                                                    Point(0., -1. * _side_length / 2., 1.)}};
  std::vector<std::string> surf_names{"plus_x", "minus_x", "plus_y", "minus_y"};

  // initialize cell region to be updated
  CSG::CSGRegion region;
  for (unsigned int i = 0; i < points_on_planes.size(); ++i)
  {
    const auto surf_name = "surf_" + surf_names[i];
    std::unique_ptr<CSG::CSGSurface> plane_ptr = std::make_unique<CSG::CSGPlane>(
        surf_name, points_on_planes[i][0], points_on_planes[i][1], points_on_planes[i][2]);
    auto & csg_plane = csg_obj->addSurface(std::move(plane_ptr));
    const auto region_direction = csg_plane.getHalfspaceFromPoint(centroid);
    auto halfspace =
        ((region_direction == CSG::CSGSurface::Halfspace::POSITIVE) ? +csg_plane : -csg_plane);
    // check if first halfspace to be added to the region
    if (region.getRegionType() == CSG::CSGRegion::RegionType::EMPTY)
      region = halfspace;
    else
      region &= halfspace;
  }

  const auto cell_name = "square_cell";
  const auto material_name = "square_material";
  csg_obj->createCell(cell_name, material_name, region);

  return csg_obj;
}
