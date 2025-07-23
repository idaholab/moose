//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGAmbiguousHalfspaceError.h"
#include "CSGBase.h"
#include "CSGPlane.h"

registerMooseObject("MooseTestApp", TestCSGAmbiguousHalfspaceError);

InputParameters
TestCSGAmbiguousHalfspaceError::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("side_length", "Side length of infinite square.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGAmbiguousHalfspaceError::TestCSGAmbiguousHalfspaceError(const InputParameters & params)
  : MeshGenerator(params), _side_length(getParam<Real>("side_length"))
{
}

std::unique_ptr<MeshBase>
TestCSGAmbiguousHalfspaceError::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGAmbiguousHalfspaceError::generateCSG()
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // Add surfaces and halfspaces corresponding to 4 planes of infinite square
  std::vector<Point> points_on_plane{Point(1. * _side_length / 2., 0., 0.),
                                     Point(1. * _side_length / 2., 1., 0.),
                                     Point(1. * _side_length / 2., 0., 1.)};

  CSG::CSGRegion region;
  const auto surf_name = "surf_plus_x";
  std::unique_ptr<CSG::CSGSurface> plane_ptr = std::make_unique<CSG::CSGPlane>(
      surf_name, points_on_plane[0], points_on_plane[1], points_on_plane[2]);
  auto & csg_plane = csg_obj->addSurface(plane_ptr);

  // This should error as point used to evaluate halfspace lies on the plane
  csg_plane.getHalfspaceFromPoint(points_on_plane[0]);

  return csg_obj;
}
