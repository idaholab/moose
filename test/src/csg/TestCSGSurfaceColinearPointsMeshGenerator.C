//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGSurfaceColinearPointsMeshGenerator.h"
#include "CSGBase.h"

registerMooseObject("MooseTestApp", TestCSGSurfaceColinearPointsMeshGenerator);

InputParameters
TestCSGSurfaceColinearPointsMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGSurfaceColinearPointsMeshGenerator::TestCSGSurfaceColinearPointsMeshGenerator(
    const InputParameters & params)
  : MeshGenerator(params)
{
}

std::unique_ptr<MeshBase>
TestCSGSurfaceColinearPointsMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGSurfaceColinearPointsMeshGenerator::generateCSG()
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // Create a plane from collinear points
  const auto & p1 = Point(0, 0, 0);
  const auto & p2 = Point(1, 0, 0);
  const auto & p3 = Point(2, 0, 0);
  const auto & surf_name = "surf";
  csg_obj->createPlaneFromPoints(surf_name, p1, p2, p3);

  return csg_obj;
}
