/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "StitchedMesh.h"
#include "Parser.h"
#include "InputParameters.h"

// libMesh includes
#include "libmesh/mesh_modification.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"

template <>
InputParameters
validParams<StitchedMesh>()
{
  InputParameters params = validParams<MooseMesh>();
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

  // Get the original mesh
  _original_mesh = dynamic_cast<ReplicatedMesh *>(&getMesh());
  if (!_original_mesh)
    mooseError("StitchedMesh does not support DistributedMesh");

  // Read the first mesh into the original mesh... then we'll stitch all of the others into that
  _original_mesh->read(_files[0]);

  _meshes.reserve(_files.size() - 1);

  // Read in all of the other meshes
  for (auto i = beginIndex(_files, 1); i < _files.size(); ++i)
  {
    _meshes.emplace_back(libmesh_make_unique<ReplicatedMesh>(_communicator));
    auto & mesh = _meshes.back();

    mesh->read(_files[i]);
  }

  if (_stitch_boundaries.size() % 2 != 0)
    mooseError("There must be an even amount of stitch_boundaries in ", name());

  _stitch_boundaries_pairs.reserve(_stitch_boundaries.size() / 2);

  // Make pairs out of the boundary names
  for (auto i = beginIndex(_stitch_boundaries); i < _stitch_boundaries.size(); i += 2)
    _stitch_boundaries_pairs.emplace_back(_stitch_boundaries[i], _stitch_boundaries[i + 1]);
}

StitchedMesh::StitchedMesh(const StitchedMesh & other_mesh)
  : MooseMesh(other_mesh),
    _files(other_mesh._files),
    _clear_stitched_boundary_ids(other_mesh._clear_stitched_boundary_ids),
    _stitch_boundaries(other_mesh._stitch_boundaries)
{
}

MooseMesh &
StitchedMesh::clone() const
{
  return *(new StitchedMesh(*this));
}

void
StitchedMesh::buildMesh()
{
  // Stich 'em
  for (auto i = beginIndex(_meshes); i < _meshes.size(); i++)
  {
    auto & boundary_pair = _stitch_boundaries_pairs[i];

    BoundaryID first = getBoundaryID(boundary_pair.first);
    BoundaryID second = getBoundaryID(boundary_pair.second);

    _original_mesh->stitch_meshes(
        *_meshes[i], first, second, TOLERANCE, _clear_stitched_boundary_ids);
  }
}
