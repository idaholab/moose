//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGSphereAtXYZMeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGSphereAtXYZMeshGenerator);

InputParameters
TestCSGSphereAtXYZMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("radius", "radius of sphere.");
  params.addRequiredParam<Real>("x0", "x coord of center point of sphere.");
  params.addRequiredParam<Real>("y0", "y coord of center point of sphere.");
  params.addRequiredParam<Real>("z0", "z coord of center point of sphere.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGSphereAtXYZMeshGenerator::TestCSGSphereAtXYZMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _radius(getParam<Real>("radius")),
    _x0(getParam<Real>("x0")),
    _y0(getParam<Real>("y0")),
    _z0(getParam<Real>("z0"))
{
}

std::unique_ptr<MeshBase>
TestCSGSphereAtXYZMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGSphereAtXYZMeshGenerator::generateCSG()
{
  auto csg_mesh = std::make_unique<CSG::CSGBase>();

  std::string root_univ_name = "root_sphere_point";
  auto root_univ = csg_mesh->createRootUniverse(root_univ_name);

  csg_mesh->createSphereAtXYZ("sphere_surf", _x0, _y0, _z0, _radius);
  // TODO: make cells: auto elem_cell_ptr = root_univ->addMaterialCell(cell_name, material_name);

  return csg_mesh;
}
