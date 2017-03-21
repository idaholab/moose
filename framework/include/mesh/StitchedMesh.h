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

#ifndef STITCHEDMESH_H
#define STITCHEDMESH_H

#include "MooseMesh.h"

#include "libmesh/serial_mesh.h"

class StitchedMesh;

template <>
InputParameters validParams<StitchedMesh>();

/**
 * Reads an arbitrary set of meshes and attempts to "stitch" (join) them
 * along boundaries.
 */
class StitchedMesh : public MooseMesh
{
public:
  StitchedMesh(const InputParameters & parameters);
  StitchedMesh(const StitchedMesh & other_mesh);

  virtual MooseMesh & clone() const override;

  virtual void buildMesh() override;

protected:
  /// The mesh files to read
  const std::vector<MeshFileName> & _files;

  /// Whether or not to clear (remove) the stitched boundary IDs
  const bool & _clear_stitched_boundary_ids;

  /// The raw data from the input file
  const std::vector<BoundaryName> & _stitch_boundaries;

  /// A transformed version of _stitch_boundaries into a more logical "pairwise" structure
  std::vector<std::pair<BoundaryName, BoundaryName>> _stitch_boundaries_pairs;

  // Pointer to the original "real" mesh to be stitched into
  ReplicatedMesh * _original_mesh;

  /// The meshes to be stitched together.  The first entry will be the "real" mesh
  std::vector<std::unique_ptr<ReplicatedMesh>> _meshes;
};

#endif /* STITCHEDMESH_H */
