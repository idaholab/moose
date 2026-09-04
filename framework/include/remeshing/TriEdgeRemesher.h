//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Remesher.h"

#include "libmesh/dof_object.h"

#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

class MooseVariableFieldBase;

/**
 * Remesher that adapts a TRI3 mesh toward a target element size field by local edge operations:
 * splitting the edges that are long against the target, collapsing the ones that are short, and
 * swapping the ones whose swap improves element shape.
 *
 * Every edge is measured in the metric the target size field induces: its length divided by the
 * target size interpolated to it. An edge longer than sqrt(2) in that measure is split at its
 * midpoint, and an edge shorter than 1/sqrt(2) is collapsed by removing one of its vertices, the
 * classical bounds that keep the two operations out of each other's output: the children of a
 * split edge land above the collapse bound, and a collapse is refused when it would create an edge
 * above the split bound. An edge between those bounds is at rest, which is what lets a mesh that
 * meets its target pass through an event untouched. Refinement and coarsening therefore come out
 * of one operator set and one field, where the splitting and the patch Delaunay remeshers divide
 * the same work between two objects.
 *
 * The operations are performed on a shadow copy of the mesh rather than on the mesh itself,
 * because the Remesher contract keeps every replaced element in the mesh until the engine has read
 * the old solution through it, and chaining local operations requires the intermediate states to
 * be mutable. The shadow starts as the current mesh, the passes mutate it, and remesh() splices
 * the difference: an element whose shadow triangle survives every pass is left alone, and the
 * others are replaced. Every operation is arithmetic on nodes and midpoints already decided, and
 * the candidate edges of every pass are processed in a sorted order, so a repeated run reaches the
 * same mesh with the same element count, which the Delaunay path cannot promise.
 *
 * Exterior boundaries, sidesets and subdomain seams pass through the surgery unchanged: a vertex
 * on any of them is never removed, an edge on any of them is never swapped, and a collapse never
 * reaches across one. Such an edge may still be split, and its midpoint inherits its boundary ids,
 * so refinement tracks a moving boundary the way the splitting remesher does. Splitting here is a
 * plain bisection of the two adjacent triangles rather than a red-green pattern: the midpoint
 * bisects both neighbors by construction, so no closure is needed and no hanging node is possible.
 *
 * The solution transfer follows the patch Delaunay contract: every new node and every new triangle
 * centroid lies inside the union of the elements the event replaces, so its source is located
 * among them while they are still in the mesh. Every operation records which of those elements
 * cover what it builds, so the search is confined to a few elements rather than to everything the
 * event replaced.
 *
 * This first version runs on replicated meshes only, where every rank holds the whole mesh and
 * performs the identical surgery. The target size is only readable on the elements a rank owns, so
 * the targets are gathered before the shadow is built, the way the patch Delaunay remesher gathers
 * them.
 */
class TriEdgeRemesher : public Remesher
{
public:
  static InputParameters validParams();

  TriEdgeRemesher(const InputParameters & parameters);

  virtual RemeshRecord remesh() override;

private:
  /// The two vertex indices of a shadow edge in increasing order, so that both triangles agree
  using EdgeKey = std::pair<std::size_t, std::size_t>;

  /**
   * One vertex of the shadow mesh: a node of the current mesh, or a midpoint a split created.
   */
  struct ShadowVertex
  {
    /// The mesh node this vertex started as, null for a midpoint a split created
    Node * node = nullptr;
    /// Position of the vertex, which no operation ever moves
    Point point;
    /// Target size at the vertex, unset where the sizing variable gave no adjacent element one
    std::optional<Real> target;
    /// Whether the vertex lies on an exterior boundary, a sideset or a subdomain seam, which a
    /// collapse may never remove
    bool locked = false;
    /// False once a collapse removed the vertex
    bool alive = true;
    /// Ids of the mesh elements whose union covers the vertex, in increasing order, empty for a
    /// mesh node: a midpoint lies on the edge it bisects, so the elements that cover the triangles
    /// on that edge cover it
    std::vector<dof_id_type> ancestors;
  };

  /**
   * One triangle of the shadow mesh, in counter-clockwise vertex order.
   */
  struct ShadowTriangle
  {
    /// Indices into the vertex vector, counter-clockwise
    std::array<std::size_t, 3> vertices;
    /// Subdomain the triangle lies in, which every operation preserves
    subdomain_id_type subdomain;
    /// Id of the mesh element this triangle started as, invalid for one an operation created
    dof_id_type original_id = DofObject::invalid_id;
    /// False once an operation replaced the triangle
    bool alive = true;
    /// Ids of the mesh elements whose union covers the triangle, in increasing order: the element
    /// it started as, or the ancestors of every triangle the operation that created it consumed
    std::vector<dof_id_type> ancestors;
  };

  /**
   * The whole shadow mesh one event mutates, with the adjacency the operations maintain.
   */
  struct ShadowMesh
  {
    std::vector<ShadowVertex> vertices;
    std::vector<ShadowTriangle> triangles;
    /// The alive triangles on each edge, one for a boundary edge and two for an interior one
    std::map<EdgeKey, std::vector<std::size_t>> edge_triangles;
    /// The alive triangles around each vertex
    std::vector<std::set<std::size_t>> vertex_triangles;
    /// Boundary ids of the edges that carry any, maintained through splits
    std::map<EdgeKey, std::vector<boundary_id_type>> edge_boundary_ids;
    /// Ids of the mesh elements some operation replaced
    std::set<dof_id_type> replaced_element_ids;
  };

  /// The sorted pair key of vertices \p a and \p b
  static EdgeKey edgeKey(std::size_t a, std::size_t b);

  /**
   * Gather the target size of every element into a table keyed by element id. Only the owner of an
   * element can read the target off the solution, and every rank performs the whole surgery, so
   * the owned readings are gathered the way the patch Delaunay remesher gathers them.
   *
   * @return the table, holding every element the sizing variable is defined on
   */
  std::map<dof_id_type, Real> buildTargetSizeTable() const;

  /**
   * Build the shadow copy of the current mesh: one vertex per node in increasing node id order,
   * one triangle per element in increasing element id order, the adjacency, the boundary ids the
   * edges carry, the per-vertex target sizes, and the locked flags.
   *
   * @param target_sizes the gathered target size table
   * @return the shadow mesh the passes mutate
   */
  ShadowMesh buildShadowMesh(const std::map<dof_id_type, Real> & target_sizes) const;

  /**
   * The length of edge (\p a, \p b) measured in the metric of the target size field, which is the
   * Euclidean length divided by the mean of the targets at the two vertices.
   *
   * @return the metric length, or nothing when neither vertex carries a target, which leaves the
   * edge out of every size-driven pass
   */
  static std::optional<Real> metricLength(const ShadowMesh & shadow, std::size_t a, std::size_t b);

  /// Whether edge (\p a, \p b) may not be swapped or collapsed across: it lies on an exterior
  /// boundary, carries boundary ids, or separates two subdomains
  static bool edgeConstrained(const ShadowMesh & shadow, const EdgeKey & key);

  /// The signed area of the triangle (\p a, \p b, \p c), positive for counter-clockwise winding
  static Real signedArea(const Point & a, const Point & b, const Point & c);

  /// The shape quality of the triangle (\p a, \p b, \p c): 4*sqrt(3)*area over the sum of the
  /// squared edge lengths, which is 1 for an equilateral triangle and 0 for a degenerate one
  static Real shapeQuality(const Point & a, const Point & b, const Point & c);

  /// The shape quality of shadow triangle \p t
  static Real triangleQuality(const ShadowMesh & shadow, const ShadowTriangle & t);

  /// Add triangle (\p v0, \p v1, \p v2) to the shadow and index it, inheriting \p subdomain and
  /// covered by the mesh elements \p ancestors
  static std::size_t addShadowTriangle(ShadowMesh & shadow,
                                       std::size_t v0,
                                       std::size_t v1,
                                       std::size_t v2,
                                       subdomain_id_type subdomain,
                                       const std::vector<dof_id_type> & ancestors);

  /// Merge the sorted ids \p more into the sorted ids \p ancestors, without duplicates
  static void mergeAncestors(std::vector<dof_id_type> & ancestors,
                             const std::vector<dof_id_type> & more);

  /// Remove triangle \p t from the shadow and its adjacency, recording the mesh element it
  /// replaced when it was an original one
  static void removeShadowTriangle(ShadowMesh & shadow, std::size_t t);

  /**
   * Split every edge whose metric length exceeds sqrt(2), at its midpoint, bisecting the one or
   * two triangles on it. The candidates are collected up front and processed in increasing edge
   * key order; each is re-measured when its turn comes, because an earlier split can have replaced
   * a triangle on it.
   *
   * @param shadow the shadow mesh, mutated in place
   * @return the number of edges split
   */
  unsigned int splitPass(ShadowMesh & shadow) const;

  /**
   * Collapse every edge whose metric length is below 1/sqrt(2), where a valid collapse exists,
   * removing one vertex and retargeting its triangles at the other. For each candidate both
   * directions are tried in increasing vertex index order, and a direction is valid when the
   * removed vertex is not locked, the link of the edge is exactly the two opposite vertices, every
   * retargeted triangle keeps counter-clockwise winding and acceptable shape, and no retargeted
   * edge leaves the event longer than the split bound.
   *
   * @param shadow the shadow mesh, mutated in place
   * @return the number of edges collapsed
   */
  unsigned int collapsePass(ShadowMesh & shadow) const;

  /**
   * Swap every unconstrained interior edge whose swap strictly improves the worse shape quality of
   * its two triangles, processed in increasing edge key order. A swap is valid when the
   * quadrilateral around the edge is strictly convex and the swapped edge does not already exist.
   *
   * @param shadow the shadow mesh, mutated in place
   * @return the number of edges swapped
   */
  unsigned int swapPass(ShadowMesh & shadow) const;

  /**
   * Create the new nodes and triangles the surviving shadow describes, and record everything the
   * event replaced. The source of every new node and of every new triangle centroid is located
   * among the replaced elements that cover it, which are still in the mesh, following the patch
   * Delaunay transfer contract.
   *
   * @param shadow the shadow mesh after the last pass
   * @param record the record the new and the replaced entities are appended to
   */
  void spliceShadow(const ShadowMesh & shadow, RemeshRecord & record) const;

  /// Target element size field, a CONSTANT MONOMIAL variable read as one value per element
  const MooseVariableFieldBase & _sizing_variable;

  /// Floor the target element size is held at, unset when the sizing variable alone carries it
  const std::optional<Real> _min_element_size;

  /// Number of split-collapse-swap rounds one event performs
  const unsigned int _max_iterations;
};

inline TriEdgeRemesher::EdgeKey
TriEdgeRemesher::edgeKey(const std::size_t a, const std::size_t b)
{
  return a < b ? std::make_pair(a, b) : std::make_pair(b, a);
}
