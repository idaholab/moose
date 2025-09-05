//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGSphereAtOriginMeshGenerator.h"
#include "CSGSphere.h"

registerMooseObject("MooseTestApp", TestCSGSphereAtOriginMeshGenerator);

InputParameters
TestCSGSphereAtOriginMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("radius", "radius of sphere.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGSphereAtOriginMeshGenerator::TestCSGSphereAtOriginMeshGenerator(
    const InputParameters & params)
  : MeshGenerator(params), _radius(getParam<Real>("radius"))
{
}

std::unique_ptr<MeshBase>
TestCSGSphereAtOriginMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGSphereAtOriginMeshGenerator::generateCSG()
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto mg_name = this->name();

  // create a sphere surface of the specified radius at the origin
  std::unique_ptr<CSG::CSGSurface> sp_ptr =
      std::make_unique<CSG::CSGSphere>(mg_name + "_sphere_surf", _radius);
  auto & sphere_surf = csg_obj->addSurface(std::move(sp_ptr));
  csg_obj->createCell(mg_name + "_sphere_cell", -sphere_surf);

  return csg_obj;
}
