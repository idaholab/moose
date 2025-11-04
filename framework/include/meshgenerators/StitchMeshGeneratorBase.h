//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * A base class for mesh generators that stitch boundaries together
 */
class StitchMeshGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  StitchMeshGeneratorBase(const InputParameters & parameters);

protected:
  /**
   * @brief Get the boundary id from the name of a boundary to stitch
   * @param mesh the mesh the boundary should be a part of
   * @param input_mg_name the name of the mesh generator creating that mesh
   * @param bname the name of the boundary
   */
  boundary_id_type getBoundaryIdToStitch(const MeshBase & mesh,
                                         const std::string & input_mg_name,
                                         const BoundaryName & bname) const;

  /**
   * @brief Returns an error if the boundary to be stitched does not exist
   * @param mesh the mesh the boundary should be a part of
   * @param input_mg_name the name of the mesh generator creating that mesh
   * @param bname the name of the boundary
   */
  void errorMissingBoundary(const MeshBase & mesh,
                            const std::string & input_mg_name,
                            const BoundaryName & bname) const;

  /// Whether or not to clear (remove) the stitched boundary IDs
  const bool _clear_stitched_boundary_ids;

  /// A transformed version of _stitch_boundaries into a more logical "pairwise" structure
  const std::vector<std::vector<std::string>> _stitch_boundaries_pairs;

  /// Type of algorithm used to find matching nodes (binary or exhaustive)
  const MooseEnum _algorithm;
};
