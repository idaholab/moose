//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StitchedMesh.h"
#include "Parser.h"
#include "InputParameters.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"

registerMooseObject("MooseApp", StitchedMesh);

InputParameters
StitchedMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addRequiredParam<std::vector<MeshFileName>>(
      "files",
      "The name of the mesh files to read.  These mesh files will be 'stitched' into the "
      "current mesh in this order.");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "stitch_boundaries",
      "Pairs of boundary names (one after the other) to stitch together for each step.");

  params.addParam<bool>(
      "clear_stitched_boundary_ids",
      true,
      "Whether or not to erase the boundary IDs after they've been used for stitching.");

  params.addClassDescription(
      "Reads in all of the given meshes and stitches them all together into one mesh.");

  return params;
}

StitchedMesh::StitchedMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _files(getParam<std::vector<MeshFileName>>("files")),
    _clear_stitched_boundary_ids(getParam<bool>("clear_stitched_boundary_ids")),
    _stitch_boundaries(getParam<std::vector<BoundaryName>>("stitch_boundaries"))
{
  if (_files.empty())
    mooseError("Must specify at least one mesh file for StitchedMesh");

  // The StitchedMesh class only works with ReplicatedMesh
  errorIfDistributedMesh("StitchedMesh");

  if (_stitch_boundaries.size() % 2 != 0)
    mooseError("There must be an even amount of stitch_boundaries in ", name());

  _stitch_boundaries_pairs.reserve(_stitch_boundaries.size() / 2);

  // Make pairs out of the boundary names
  for (MooseIndex(_stitch_boundaries) i = 0; i < _stitch_boundaries.size(); i += 2)
    _stitch_boundaries_pairs.emplace_back(_stitch_boundaries[i], _stitch_boundaries[i + 1]);
}

StitchedMesh::StitchedMesh(const StitchedMesh & other_mesh)
  : MooseMesh(other_mesh),
    _files(other_mesh._files),
    _clear_stitched_boundary_ids(other_mesh._clear_stitched_boundary_ids),
    _stitch_boundaries(other_mesh._stitch_boundaries)
{
}

std::unique_ptr<MooseMesh>
StitchedMesh::safeClone() const
{
  return std::make_unique<StitchedMesh>(*this);
}

void
StitchedMesh::buildMesh()
{
  // Get the original mesh
  _original_mesh = static_cast<ReplicatedMesh *>(&getMesh());

  // Read the first mesh into the original mesh... then we'll stitch all of the others into that
  _original_mesh->read(_files[0]);

  _meshes.reserve(_files.size() - 1);

  // Read in all of the other meshes
  for (MooseIndex(_files) i = 1; i < _files.size(); ++i)
  {
    _meshes.emplace_back(std::make_unique<ReplicatedMesh>(_communicator));
    auto & mesh = _meshes.back();

    mesh->read(_files[i]);
  }

  // Stich 'em
  for (MooseIndex(_meshes) i = 0; i < _meshes.size(); i++)
  {
    auto & boundary_pair = _stitch_boundaries_pairs[i];

    BoundaryID first = getBoundaryID(boundary_pair.first);
    BoundaryID second = getBoundaryID(boundary_pair.second);

    _original_mesh->stitch_meshes(
        *_meshes[i], first, second, TOLERANCE, _clear_stitched_boundary_ids);
  }
}
