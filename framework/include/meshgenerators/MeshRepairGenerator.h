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
#include "MooseEnum.h"

/**
 * Mesh generator to perform various improvement / fixing operations on an input mesh
 */
class MeshRepairGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  MeshRepairGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

private:
  /// the input mesh
  std::unique_ptr<MeshBase> & _input;

  /// fixing mesh by deleting overlapping nodes
  const bool _fix_overlapping_nodes;
  /// tolerance for merging overlapping nodes
  const Real _node_overlap_tol;

  /// whether to flip element orientation such that they no longer have a negative volume
  const bool _fix_element_orientation;

  /// whether to split subdomains using each element's type
  const bool _elem_type_separation;

  /// Whether to merge boundaries with the same name but different ID
  const bool _boundary_id_merge;

  /// Whether to split non-convex polygons
  const bool _split_nonconvex_polygons;

  /// whether to repair sliver (near-degenerate) first-order 2D elements
  const bool _fix_sliver_elements;
  /// a 2D element is a sliver if its area is below this fraction of the mesh area (0 disables)
  const Real _sliver_area_tol;
  /// a 2D element is a sliver if every non-longest-edge vertex is within this fraction of the
  /// longest-edge length from that edge (0 disables)
  const Real _sliver_flap_tol;

  /// @brief Removes the elements with an volume value below the user threshold
  /// @param mesh the mesh to modify
  void removeSmallVolumeElements(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Removes nodes that overlap
  /// @param mesh the mesh to modify
  void fixOverlappingNodes(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Separate subdomain by element type because some output format (Exodus)
  ///        do not support mixed element types
  /// @param mesh the mesh to modify
  void separateSubdomainsByElementType(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Splits non-convex polygonal elements to keep only convex elements
  /// @param mesh the mesh to modify
  void splitNonConvexPolygons(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Repair sliver (near-degenerate) first-order 2D elements (TRI3, QUAD4, polygons). Each
  ///        sliver is removed and absorbed into its longest-edge neighbor, keeping the surface
  ///        conformal (no holes or hanging nodes). A triangle sliver against a triangle neighbor
  ///        splits that neighbor into two triangles; otherwise the neighbor absorbs the sliver's
  ///        vertices and is promoted to a quad or polygon.
  /// @param mesh the mesh to modify
  void repairSlivers(std::unique_ptr<MeshBase> & mesh) const;
};
