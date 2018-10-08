//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StitchedMeshGenerator.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"

registerMooseObject("MooseApp", StitchedMeshGenerator);

template <>
InputParameters
validParams<StitchedMeshGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<std::vector<MeshFileName>>("files_names", "The mesh files to read.");
  params.addParam<bool>("clear_stitched_boundary_ids", true, "Whether or not to clear the stitched boundary IDs");
  params.addRequiredParam< std::vector<std::vector<std::string>> >("stitch_boundaries_pairs", "Pairs of boundaries to be stitched together");

  return params;
}

StitchedMeshGenerator::StitchedMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
  _files_names(getParam<std::vector<MeshFileName>>("files_names")),
  _clear_stitched_boundary_ids(getParam<bool>("clear_stitch_boundary_ids")),
  _stitch_boundaries_pairs(getParam<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs"))
{
}

std::unique_ptr<MeshBase>
StitchedMeshGenerator::generate()
{
  auto mesh = libmesh_make_unique<ReplicatedMesh>(comm());

  mesh->read(_files_names[0]);

  _meshes.reserve(_files_names.size() - 1);

  // Read in all of the other meshes
  for (auto i = beginIndex(_files_names, 1); i < _files_names.size(); ++i)
  {
    _meshes.emplace_back(libmesh_make_unique<ReplicatedMesh>(comm()));
    _meshes.back()->read(_files_names[i]);
  }

  // Stich 'em
  for (auto i = beginIndex(_meshes); i < _meshes.size(); i++)
  {
    auto & boundary_pair = _stitch_boundaries_pairs[i];

    boundary_id_type first =  _meshes[i]->get_id_by_name(boundary_pair[0]);
    boundary_id_type second = _meshes[i]->get_id_by_name(boundary_pair[1]);

    mesh->stitch_meshes(*_meshes[i], first, second, TOLERANCE, _clear_stitched_boundary_ids);
  }

  return mesh;
}
