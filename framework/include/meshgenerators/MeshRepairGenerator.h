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

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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
  /// a TET4 is a sliver if its volume is below this fraction of the mesh bounding-box volume
  /// (0 disables)
  const Real _sliver_volume_tol;
  /// relative floor below which a collapse-reshaped tet is rejected as inverting / re-slivering
  const Real _tet_collapse_volume_floor;

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

  /// @brief Repair sliver (near-degenerate) TET4 elements by edge collapse. Each sliver is removed
  ///        by collapsing one of its edges (merging a node onto another existing node), keeping a
  ///        valid all-tetrahedral, conformal, manifold mesh. A candidate collapse is committed only
  ///        if it does not invert/degenerate any neighbor, does not create a non-manifold
  ///        configuration, and does not distort the mesh boundary; otherwise the sliver is left in
  ///        place. Repairs run in node-disjoint passes.
  /// @param mesh the mesh to modify
  void repairTetSlivers(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Repair sliver (near-degenerate, flat) PYRAMID5 elements by absorbing each into the
  ///        element sharing its quad base. The shared quad face is dissolved and the neighbor (a
  ///        hex, prism, polyhedron, or another pyramid) is replaced by a C0Polyhedron made of its
  ///        remaining faces plus the sliver pyramid's four triangular side faces. No node is moved,
  ///        so the surrounding elements stay conformal. A pyramid is left in place if it has no
  ///        element across its quad base or the resulting polyhedron would be invalid.
  /// @param mesh the mesh to modify
  void repairPyramidSlivers(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Repair sliver (near-degenerate) PRISM6 (wedge) elements. A flat (axially squashed)
  ///        wedge is repaired by collapsing its top triangle onto its bottom triangle so the
  ///        elements above and below it meet; a thin-cross-section (blade) wedge is absorbed into
  ///        the element across its longest quad side, which becomes a C0Polyhedron. A wedge is left
  ///        in place if no valid repair exists (the collapse would invert/degenerate a neighbor or
  ///        distort the boundary, or the absorbed union would be an invalid cell).
  /// @param mesh the mesh to modify
  void repairWedgeSlivers(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Absorb a sliver element into the neighbor sharing the face with sorted node-id key
  ///        @p shared_key, by replacing both with a single C0Polyhedron whose faces are both
  ///        elements' faces except the shared one. Side and edge boundary ids and the neighbor's
  ///        subdomain are carried onto the polyhedron. On success the polyhedron is added, both
  ///        elements are deleted, their nodes are recorded in @p touched_nodes, and true is
  ///        returned; if the union is not a valid convex cell the mesh is left unchanged and false
  ///        is returned.
  bool absorbAcrossSharedFace(std::unique_ptr<MeshBase> & mesh,
                              Elem * sliver,
                              Elem * neighbor,
                              const std::vector<dof_id_type> & shared_key,
                              std::unordered_set<dof_id_type> & touched_nodes) const;

  /// @brief Collapse a sliver element by merging each @p gone_kept node pair (moving the first node
  ///        onto the second), e.g. one flat cap face of a wedge or hexahedron onto its opposite.
  ///        The sliver is deleted and its neighbors stay valid: the merge is committed only if it
  ///        leaves every other element in the collapse star non-degenerate and non-inverted (volume
  ///        above @p invert_floor), otherwise the mesh is left unchanged. The caller is responsible
  ///        for capturing the gone node pointers up front and for ensuring the connecting side faces
  ///        are unshared. Returns true and records the affected nodes in @p touched_nodes on commit.
  bool collapseByFaceMerge(
      std::unique_ptr<MeshBase> & mesh,
      Elem * sliver,
      const std::vector<std::pair<Node *, Node *>> & gone_kept,
      const std::unordered_map<dof_id_type, std::vector<dof_id_type>> & node_to_elems,
      std::unordered_set<dof_id_type> & touched_nodes,
      Real invert_floor) const;

  /// @brief Repair sliver (near-degenerate, flat-slab) HEX8 elements by collapsing the squashed
  ///        pair of opposite faces together so the elements on either side meet. A hex is left in
  ///        place if no pair is sufficiently squashed, a connecting side face is shared, or the
  ///        collapse would invert/degenerate a neighbor.
  /// @param mesh the mesh to modify
  void repairHexSlivers(std::unique_ptr<MeshBase> & mesh) const;
};
