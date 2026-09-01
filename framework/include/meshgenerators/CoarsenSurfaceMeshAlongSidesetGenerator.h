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

/**
 * MeshGenerator that coarsens a 2D-element (TRI3/QUAD4) surface mesh along a sideset by
 * collapsing alternate boundary nodes. The sideset may be internal: elements on both sides
 * of it are coarsened and the sideset itself is preserved.
 */
class CoarsenSurfaceMeshAlongSidesetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  CoarsenSurfaceMeshAlongSidesetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /**
   * Performs a single coarsening pass: collapse a maximal independent set of sideset nodes,
   * merging pairs of elements.
   * @return the number of nodes collapsed during the pass
   */
  unsigned int coarsenAlongSidesets(std::unique_ptr<MeshBase> & mesh,
                                    const std::set<boundary_id_type> & boundary_id_set);

  /// Input mesh to coarsen
  std::unique_ptr<MeshBase> & _input;

  /// Sideset(s) to coarsen the mesh along
  const std::vector<BoundaryName> _boundaries;

  /// Sideset(s) to exclude when coarsening along all the sidesets of the mesh
  const std::vector<BoundaryName> _exclude_boundaries;

  /// Whether a maximum normal deviation between merged elements is enforced
  const bool _has_max_normal_deviation;
  /// Maximum angle (degrees) between the normals of the two elements merged together
  const Real _max_normal_deviation;

  /// Whether a maximum merged side length is enforced
  const bool _has_max_side_length;
  /// Maximum length of the side created by merging two elements
  const Real _max_merged_side_length;

  /// Whether a maximum merged element area is enforced
  const bool _has_max_element_area;
  /// Maximum area of an element created by merging two elements
  const Real _max_merged_element_area;

  /// Whether to repeat the coarsening pass so that more than two elements can be merged together
  const bool _coarsen_more_than_two_elements;

  /// Whether the mesh generator should be verbose to the console
  const bool _verbose;
};
