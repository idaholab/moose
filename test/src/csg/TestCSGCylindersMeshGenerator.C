//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGCylindersMeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGCylindersMeshGenerator);

InputParameters
TestCSGCylindersMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("radius", "radius of cylinder.");
  params.addRequiredParam<Real>("x0", "first coordinate of center.");
  params.addRequiredParam<Real>("x1", "second coordinate of center.");
  params.addRequiredParam<std::string>("axis", "axis alignment");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGCylindersMeshGenerator::TestCSGCylindersMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _radius(getParam<Real>("radius")),
    _x0(getParam<Real>("x0")),
    _x1(getParam<Real>("x1")),
    _axis(getParam<std::string>("axis"))
{
}

std::unique_ptr<MeshBase>
TestCSGCylindersMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGCylindersMeshGenerator::generateCSG()
{
  auto csg_mesh = std::make_unique<CSG::CSGBase>();

  csg_mesh->createCylinder("cylinder_surf_" + _axis, _x0, _x1, _radius, _axis);
  // TODO: make cells: auto elem_cell_ptr = root_univ->addMaterialCell(cell_name, material_name);

  return csg_mesh;
}
