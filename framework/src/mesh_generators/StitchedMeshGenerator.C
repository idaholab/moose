//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StitchedMeshGenerator.h"

#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"

registerMooseObject("MooseApp", StitchedMeshGenerator);

template <>
InputParameters
validParams<StitchedMeshGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  params.addParam<bool>("clear_stitched_boundary_ids", true, "Whether or not to clear the stitchd boundary IDs");
  params.addRequiredParam< std::vector<std::vector<std::string>> >("stitch_boundaries_pairs", "Pairs of boundaries to be stitched together");

  return params;
}

StitchedMeshGenerator::StitchedMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _clear_stitched_boundary_ids(getParam<bool>("clear_stitched_boundary_ids")),
    _stitch_boundaries_pairs(getParam<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs"))
{
  // Grab the input meshes
  _mesh_ptrs.reserve(_input_names.size());
  for (auto i = beginIndex(_input_names); i < _input_names.size(); ++i)
    _mesh_ptrs.push_back(&getMeshByName(_input_names[i]));
}

std::unique_ptr<MeshBase>
StitchedMeshGenerator::generate()
{
  // We put the first mesh in a local pointer
  std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<ReplicatedMesh>(*_mesh_ptrs[0]);

  // Reserve spaces for the other meshes (no need to store the first one another time)
  _meshes.reserve(_input_names.size() - 1);

  // Read in all of the other meshes
  for (auto i = beginIndex(_input_names, 1); i < _input_names.size(); ++i)
  {
    _meshes.push_back(dynamic_pointer_cast<ReplicatedMesh>(*_mesh_ptrs[i]));
  }

  // Stich all the meshes to the first one
  for (auto i = beginIndex(_meshes); i < _meshes.size(); i++)
  {
    auto boundary_pair = _stitch_boundaries_pairs[i];

    boundary_id_type first = _meshes[i]->get_boundary_info().get_id_by_name(boundary_pair[0]);
    boundary_id_type second = _meshes[i]->get_boundary_info().get_id_by_name(boundary_pair[1]);

    mesh->stitch_meshes(*_meshes[i], first, second, TOLERANCE, _clear_stitched_boundary_ids);
  }
  
  
  return mesh;
}
