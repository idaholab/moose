//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "libmesh/replicated_mesh.h"
#include "MooseEnum.h"

/**
 * Allows a pair of boundaries to be "stitched" together.
 */
class StitchBoundaryMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  StitchBoundaryMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the input mesh
  std::unique_ptr<MeshBase> & _input;

  /// Whether or not to clear (remove) the stitched boundary IDs
  const bool & _clear_stitched_boundary_ids;

  /// Pair of stitch boundaries
  std::vector<boundary_id_type> _stitch_boundaries_pair;

  /// Type of algorithm used to find matching nodes (binary or exhaustive)
  MooseEnum _algorithm;
};
