//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMMeshGeneratorMesh.h"

registerMooseObject("ThermalHydraulicsApp", THMMeshGeneratorMesh);

InputParameters
THMMeshGeneratorMesh::validParams()
{
  InputParameters params = MeshGeneratorMesh::validParams();

  params.set<bool>("allow_renumbering") = false;

  params.addClassDescription(
      "Mesh generated using mesh generators for the thermal hydraulics module.");

  return params;
}

THMMeshGeneratorMesh::THMMeshGeneratorMesh(const InputParameters & parameters)
  : MeshGeneratorMesh(parameters)
{
}

unsigned int
THMMeshGeneratorMesh::dimension() const
{
  return 3;
}

unsigned int
THMMeshGeneratorMesh::effectiveSpatialDimension() const
{
  return 3;
}

std::unique_ptr<MooseMesh>
THMMeshGeneratorMesh::safeClone() const
{
  return std::make_unique<THMMeshGeneratorMesh>(*this);
}
