//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMMeshGenerator.h"
#include "ThermalHydraulicsApp.h"
#include "THMMesh.h"

registerMooseObject("ThermalHydraulicsApp", THMMeshGenerator);

InputParameters
THMMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += THMAppInterface::validParams();

  params.addClassDescription("Copies a THMMesh into a mesh generator.");

  return params;
}

THMMeshGenerator::THMMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), THMAppInterface(parameters)
{
}

std::unique_ptr<MeshBase>
THMMeshGenerator::generate()
{
  MeshBase & mesh_base = getTHMApp().getTHMMesh()->getMesh();

  std::unique_ptr<MeshBase> mesh;
  if (dynamic_cast<DistributedMesh *>(&mesh_base))
    mesh = std::make_unique<DistributedMesh>(static_cast<DistributedMesh &>(mesh_base));
  else if (dynamic_cast<ReplicatedMesh *>(&mesh_base))
    mesh = std::make_unique<ReplicatedMesh>(static_cast<ReplicatedMesh &>(mesh_base));
  else
    mooseError("Mesh base must be either DistributedMesh or ReplicatedMesh.");

  mesh->prepare_for_use();

  return mesh;
}
