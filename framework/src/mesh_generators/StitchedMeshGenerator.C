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
  params.addParam<bool>("clear_stitched_boundary_ids", true, "Whether or not to clear the stitched boundary IDs");
  params.addRequiredParam< std::vector<std::vector<std::string>> >("stitch_boundaries_pairs", "Pairs of boundaries to be stitched together");

  return params;
}

StitchedMeshGenerator::StitchedMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
  _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
  _clear_stitched_boundary_ids(getParam<bool>("clear_stitch_boundary_ids")),
  _stitch_boundaries_pairs(getParam<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs"))
{
}

std::unique_ptr<MeshBase>
StitchedMeshGenerator::generate()
{
  //std::unique_ptr<ReplicatedMesh> mesh = std::move(std::dynamic_pointer_cast<ReplicatedMesh>(getMeshByName(_input_names[0])));
  std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<ReplicatedMesh>(getMeshByName(_input_names[0]));
  //dynamic_cast<std::unique_ptr<ReplicatedMesh>&>(*mesh);

  //mesh = getMeshByName(_names[0]))->clone();
  _meshes.reserve(_input_names.size());

  // Read in all of the other meshes
  for (auto i = beginIndex(_input_names, 0); i < _input_names.size(); ++i)
  {
    _meshes.emplace_back(libmesh_make_unique<ReplicatedMesh>(comm()));
    _meshes.back() = getMeshByName(_input_names[i])->clone();
  }

  
  // Stich 'em
  for (auto i = beginIndex(_meshes); i < _meshes.size(); i++)
  {
    auto & boundary_pair = _stitch_boundaries_pairs[i];

    boundary_id_type first =  _meshes[i]->get_id_by_name(boundary_pair[0]);
    boundary_id_type second = _meshes[i]->get_id_by_name(boundary_pair[1]);

    //_meshes[0]->stitch_meshes(*_meshes[i], first, second, TOLERANCE, _clear_stitched_boundary_ids);
  }
  
  return mesh;
}
