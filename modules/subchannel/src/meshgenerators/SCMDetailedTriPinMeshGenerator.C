//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMDetailedTriPinMeshGenerator.h"
#include "TriSubChannelMesh.h"
#include "libmesh/cell_prism6.h"

registerMooseObject("SubChannelApp", SCMDetailedTriPinMeshGenerator);
registerMooseObjectRenamed("SubChannelApp",
                           DetailedTriPinMeshGenerator,
                           "06/30/2025 24:00",
                           SCMDetailedTriPinMeshGenerator);

InputParameters
SCMDetailedTriPinMeshGenerator::validParams()
{
  InputParameters params = DetailedPinMeshGeneratorBase::validParams();
  params.addClassDescription(
      "Creates a detailed mesh of fuel pins in a triangular lattice arrangement");
  params.addRequiredParam<MeshGeneratorName>("input", "The corresponding subchannel mesh");
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel Pin rings per assembly [-]");
  return params;
}

SCMDetailedTriPinMeshGenerator::SCMDetailedTriPinMeshGenerator(const InputParameters & parameters)
  : DetailedPinMeshGeneratorBase(parameters),
    _input(getMesh("input")),
    _n_rings(getParam<unsigned int>("nrings"))
{
}

std::unique_ptr<MeshBase>
SCMDetailedTriPinMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh_base = std::move(_input);
  if (!mesh_base)
    mesh_base = buildMeshBaseObject();
  mesh_base->set_mesh_dimension(3);

  std::vector<Point> pin_centers;
  TriSubChannelMesh::rodPositions(pin_centers, _n_rings, _pitch, Point(0, 0));

  _elem_id = mesh_base->n_elem();
  for (auto & ctr : pin_centers)
    generatePin(mesh_base, ctr);

  mesh_base->subdomain_name(_block_id) = name();
  mesh_base->prepare_for_use();

  return mesh_base;
}
