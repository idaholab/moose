//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGInvalidRegion.h"
#include "CSGBase.h"
#include "CSGPlane.h"

registerMooseObject("MooseTestApp", TestCSGInvalidRegion);

InputParameters
TestCSGInvalidRegion::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<unsigned int>("region_type",
                                        "Type of invalid region to create (0 or 1).");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGInvalidRegion::TestCSGInvalidRegion(const InputParameters & params)
  : MeshGenerator(params), _region_type(getParam<unsigned int>("region_type"))
{
}

std::unique_ptr<MeshBase>
TestCSGInvalidRegion::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGInvalidRegion::generateCSG()
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // Add surfaces and halfspaces corresponding to 4 planes of infinite square
  std::vector<Point> points_on_plane{Point(1., 0., 0.), Point(1., 1., 0.), Point(0., 0., 0.)};

  const auto surf_name = "surf_plus_x";
  std::unique_ptr<CSG::CSGSurface> plane_ptr = std::make_unique<CSG::CSGPlane>(
      surf_name, points_on_plane[0], points_on_plane[1], points_on_plane[2]);
  auto & csg_plane = csg_obj->addSurface(std::move(plane_ptr));

  // Invalid region type 0: two regions passed into CSGRegion as a halfspace
  if (_region_type == 0)
    CSG::CSGRegion region = CSG::CSGRegion(+csg_plane, -csg_plane, "HALFSPACE");

  // Invalid region type 1: union defined where one of the regions is empty
  else if (_region_type == 1)
  {
    CSG::CSGRegion empty_reg;
    CSG::CSGRegion region = +csg_plane;
    region |= empty_reg;
  }

  return csg_obj;
}
