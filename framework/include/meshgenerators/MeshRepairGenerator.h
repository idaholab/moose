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

  /// whether to repair degenerate (near-zero-quality) elements: zero-volume, slivers, and pancakes
  const bool _fix_degenerate_elements;
  /// a 2D element is treated as degenerate (a zero-area element) if its area is below this fraction
  /// of the mesh surface-area scale (0 disables)
  const Real _zero_area_tol;
  /// flatness (flap) test tolerance: flags a flat pancake (or, in 2D, a sliver) when every
  /// off-feature vertex is within this fraction of the feature size from that feature (0 disables)
  const Real _flatness_tol;
  /// a 3D element is treated as degenerate (a zero-volume element) if its volume is below this
  /// fraction of the mesh bounding-box volume (0 disables)
  const Real _zero_volume_tol;
  /// relative floor below which a collapse-reshaped neighbor is rejected as inverting / re-degenerating
  const Real _tet_collapse_volume_floor;

  /// @brief Repair zero-volume elements: first-order 2D/3D elements collapsed toward a point
  ///        (small in every dimension, i.e. a tiny diameter hmax()). Each is removed by merging
  ///        all of its vertices onto one representative node - a sub-tolerance move - and deleting
  ///        it, reusing collapseByFaceMerge so the merge is committed only if every neighbor stays
  ///        non-degenerate and non-inverted; otherwise the element is left in place. Runs in
  ///        node-disjoint passes. A point-collapse that shares a face/edge with another element (a
  ///        degenerate cluster) is left in place - cluster removal is not yet implemented.
  /// @param mesh the mesh to modify
  void repairZeroVolumeElements(std::unique_ptr<MeshBase> & mesh) const;

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

  /// @brief Repair 2D first-order slivers (TRI3, QUAD4, polygons): thin/flat elements (flatness
  ///        test) and zero-area elements (area test). Each is removed and absorbed into its
  ///        longest-edge neighbor, keeping the surface conformal (no holes or hanging nodes). A
  ///        triangle sliver against a triangle neighbor splits that neighbor into two triangles;
  ///        otherwise the neighbor absorbs the sliver's vertices and is promoted to a quad or
  ///        polygon. A 2D element collapsed to a lower topology by a short edge or a colinear vertex
  ///        (a QUAD4 becoming an effective TRI3) is handled separately by repairQuadToTri.
  /// @param mesh the mesh to modify
  void repair2DSlivers(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Repair degenerate TET4 elements by edge collapse: flat pancakes (apex flat against the
  ///        largest face), needle slivers, and zero-volume tets are all flagged and removed the
  ///        same way. Each is removed by collapsing one of its edges (merging a node onto another
  ///        existing node), keeping a valid all-tetrahedral, conformal, manifold mesh. A candidate
  ///        collapse is committed only if it does not invert/degenerate any neighbor, does not
  ///        create a non-manifold configuration, and does not distort the mesh boundary; otherwise
  ///        the element is left in place. Repairs run in node-disjoint passes.
  /// @param mesh the mesh to modify
  void repairDegenerateTets(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Repair flat pancake PYRAMID5 elements by absorbing each into the element sharing its
  ///        quad base. The shared quad face is dissolved and the neighbor (a hex, prism, polyhedron,
  ///        or another pyramid) is replaced by a C0Polyhedron made of its remaining faces plus the
  ///        pancake pyramid's four triangular side faces. No node is moved, so the surrounding
  ///        elements stay conformal. A pyramid is left in place if it has no element across its quad
  ///        base or the resulting polyhedron would be invalid (a needle/zero-volume pyramid whose
  ///        apex projects outside the base is flagged but not absorbed). Does not handle a PYRAMID5
  ///        collapsed to a TET4 by a base-edge collapse; that is not yet implemented.
  /// @param mesh the mesh to modify
  void repairPyramidPancakes(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Repair degenerate PRISM6 (wedge) elements. A flat (axially squashed) wedge - a pancake
  ///        - is repaired by collapsing its top triangle onto its bottom triangle so the elements
  ///        above and below it meet; a thin-cross-section (blade) wedge - a sliver - is absorbed
  ///        into the element across its longest quad side, which becomes a C0Polyhedron. A wedge is
  ///        left in place if no valid repair exists (the collapse would invert/degenerate a neighbor
  ///        or distort the boundary, or the absorbed union would be an invalid cell). Does not
  ///        handle a wedge collapsed to a lower topology (short-edge degeneracy); not yet
  ///        implemented.
  /// @param mesh the mesh to modify
  void repairDegenerateWedges(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Absorb a degenerate element into the neighbor sharing the face with sorted node-id key
  ///        @p shared_key, by replacing both with a single C0Polyhedron whose faces are both
  ///        elements' faces except the shared one. Side and edge boundary ids and the neighbor's
  ///        subdomain are carried onto the polyhedron. On success the polyhedron is added, both
  ///        elements are deleted, their nodes are recorded in @p touched_nodes, and true is
  ///        returned; if the union is not a valid convex cell the mesh is left unchanged and false
  ///        is returned.
  bool absorbAcrossSharedFace(std::unique_ptr<MeshBase> & mesh,
                              Elem * degenerate,
                              Elem * neighbor,
                              const std::vector<dof_id_type> & shared_key,
                              std::unordered_set<dof_id_type> & touched_nodes) const;

  /// @brief Collapse a degenerate element by merging each @p gone_kept node pair (moving the first
  ///        node onto the second), e.g. one flat cap face of a wedge or hexahedron onto its
  ///        opposite. The element is deleted and its neighbors stay valid: the merge is committed
  ///        only if it
  ///        leaves every other element in the collapse star non-degenerate and non-inverted (volume
  ///        above @p invert_floor), otherwise the mesh is left unchanged. The caller is responsible
  ///        for capturing the gone node pointers up front and for ensuring the connecting side faces
  ///        are unshared. Returns true and records the affected nodes in @p touched_nodes on commit.
  bool collapseByFaceMerge(
      std::unique_ptr<MeshBase> & mesh,
      Elem * degenerate,
      const std::vector<std::pair<Node *, Node *>> & gone_kept,
      const std::unordered_map<dof_id_type, std::vector<dof_id_type>> & node_to_elems,
      std::unordered_set<dof_id_type> & touched_nodes,
      Real invert_floor) const;

  /// @brief Repair flat-slab pancake HEX8 elements by collapsing the squashed pair of opposite
  ///        faces together so the elements on either side meet. A hex is left in place if no pair
  ///        is sufficiently squashed (a hex thin in more than one direction - a sliver/column - or
  ///        of near-zero volume is flagged but not collapsed here), a connecting side face is
  ///        shared, or the collapse would invert/degenerate a neighbor. Does not handle a HEX8
  ///        collapsed to a PRISM6 by an edge collapse; that is not yet implemented.
  /// @param mesh the mesh to modify
  void repairHexPancakes(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Build the lower-order element that results from collapsing the edge (@p v_id, @p keep_id)
  ///        of @p e (merging v_id onto keep_id), for a supported topology reduction: QUAD4 -> TRI3,
  ///        PYRAMID5 -> TET4 (base edge only), PRISM6 -> PYRAMID5 (vertical edge only). The returned
  ///        element has keep_id in place of v_id, its nodes in a positively-oriented order, and the
  ///        subdomain id of @p e. Returns nullptr if @p e's type or the collapsed edge does not map
  ///        to a supported lower type (e.g. the two nodes are not an edge of @p e, or a pyramid
  ///        lateral edge / prism triangle edge).
  std::unique_ptr<Elem> reducedElement(const Elem & e, dof_id_type v_id, dof_id_type keep_id) const;

  /// @brief Collapse a redundant vertex @p v onto an adjacent vertex @p keep (a short edge, or a
  ///        colinear "not sticking out" vertex), reducing every incident element that contained both
  ///        to a lower topology and leaving elements that contained only @p v with @p keep in its
  ///        place. Committed only if every incident element stays valid: each reducing element must
  ///        map to a supported lower type (via reducedElement) with positive measure above
  ///        @p invert_floor, and each moved element must stay non-degenerate and above the floor;
  ///        otherwise the mesh is left unchanged. Subdomain and side/edge boundary ids are carried
  ///        onto the reduced elements, and @p v is deleted. Returns true and records the affected
  ///        nodes in @p touched_nodes on commit.
  bool collapseRedundantVertex(
      std::unique_ptr<MeshBase> & mesh,
      Node * v,
      Node * keep,
      const std::unordered_map<dof_id_type, std::vector<dof_id_type>> & node_to_elems,
      std::unordered_set<dof_id_type> & touched_nodes,
      Real invert_floor) const;

  /// @brief Repair QUAD4 elements collapsed to a triangle by a short edge or a colinear vertex, by
  ///        collapsing the redundant vertex (QUAD4 -> TRI3). A quad is left in place if the collapse
  ///        would leave a co-edge neighbor unreducible or inverted, or if a colinear vertex is not
  ///        redundant in every element sharing it (which would create a hanging node).
  /// @param mesh the mesh to modify
  void repairQuadToTri(std::unique_ptr<MeshBase> & mesh) const;
};
