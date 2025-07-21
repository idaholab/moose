//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGInputNotMovedMeshGenerator.h"
#include "MeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGInputNotMovedMeshGenerator);

InputParameters
TestCSGInputNotMovedMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The input MeshGenerator.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGInputNotMovedMeshGenerator::TestCSGInputNotMovedMeshGenerator(const InputParameters & params)
  : MeshGenerator(params), _mesh_ptr(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
TestCSGInputNotMovedMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGInputNotMovedMeshGenerator::generateCSG()
{
  static_cast<void>(getCSGBase("input"));

  // Create temporary CSG object to return
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  csg_obj->getRootUniverse();

  return csg_obj;
}
