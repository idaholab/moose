//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGOnlyMesh.h"

registerMooseObject("MooseTestApp", TestCSGOnlyMesh);

InputParameters
TestCSGOnlyMesh::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGOnlyMesh::TestCSGOnlyMesh(const InputParameters & params) : MeshGenerator(params) {}

std::unique_ptr<MeshBase>
TestCSGOnlyMesh::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

void
TestCSGOnlyMesh::generateCSG()
{
  Moose::out << "Calling generateCSG method for " << name() << "\n";
}
