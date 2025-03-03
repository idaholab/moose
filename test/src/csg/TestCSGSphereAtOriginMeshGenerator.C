//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGSphereAtOriginMeshGenerator.h"

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

TestCSGSphereAtOriginMeshGenerator::TestCSGSphereAtOriginMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _radius(getParam<Real>("radius"))
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
  auto csg_mesh = std::make_unique<CSG::CSGBase>();

  std::string root_univ_name = "root_universe";
  auto root_univ = csg_mesh->createRootUniverse(root_univ_name);

  csg_mesh->createSphereAtOrigin("sphere_surf", _radius);
  // TODO: make cells: auto elem_cell_ptr = root_univ->addMaterialCell(cell_name, material_name);

  return csg_mesh;
}
