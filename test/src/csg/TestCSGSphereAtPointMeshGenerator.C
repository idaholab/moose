//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGSphereAtPointMeshGenerator.h"
#include "CSGSphere.h"

registerMooseObject("MooseTestApp", TestCSGSphereAtPointMeshGenerator);

InputParameters
TestCSGSphereAtPointMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("radius", "radius of sphere.");
  params.addRequiredParam<Point>("center", "center point of sphere");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGSphereAtPointMeshGenerator::TestCSGSphereAtPointMeshGenerator(const InputParameters & params)
  : MeshGenerator(params), _radius(getParam<Real>("radius")), _center(getParam<Point>("center"))
{
}

std::unique_ptr<MeshBase>
TestCSGSphereAtPointMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGSphereAtPointMeshGenerator::generateCSG()
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto mg_name = this->name();
  // create a sphere surface of the specified radius at the specified center point
  std::unique_ptr<CSG::CSGSurface> sp_ptr =
      std::make_unique<CSG::CSGSphere>(mg_name + "_sphere_surf", _center, _radius);
  auto & sphere_surf = csg_obj->addSurface(sp_ptr);
  auto halfspace = sphere_surf.getHalfspaceFromPoint(_center);
  auto region = CSG::CSGRegion(sphere_surf, halfspace);
  csg_obj->createCell(mg_name + "_sphere_cell", "new_mat", region);

  return csg_obj;
}
