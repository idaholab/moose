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

#include <array>
#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

class MooseVariableFieldBase;

namespace libMesh
{
class Elem;
class Node;
}

/**
 * Remesher that splits the triangles which are larger than a target size field, and closes the
 * split so that the mesh it leaves has no hanging node.
 *
 * A triangle whose longest edge is longer than the target size on it is marked red and replaced by
 * the four triangles that the midpoints of its three edges cut it into. A midpoint on a shared edge
 * leaves the triangle on the other side of that edge non-conforming, which the red-green closure
 * repairs: a neighbor that acquires one midpoint is bisected into two through it, and a neighbor
 * that acquires two or three is promoted to a red of its own. A promotion puts midpoints on that
 * element's remaining edges, so the marking is iterated until it stops growing. No mark is ever
 * removed, which is what makes the iteration terminate.
 *
 * The target size is read off a user-authored CONSTANT MONOMIAL variable, which carries a single
 * value per element and is therefore read as one degree of freedom rather than interpolated. The
 * same field is what a size based criterion and a coarsening remesher read, so that refinement and
 * coarsening cannot pull against each other.
 *
 * The split is exact arithmetic on nodes that are already in the mesh rather than a triangulation,
 * so the entities one event produces are reproducible run to run. That only holds while nothing
 * which decides an id iterates in an unspecified order, which is why every container on that path
 * is ordered: an edge midpoint is keyed by the sorted node id pair of its edge and created once,
 * the elements are processed in increasing id order, and the ids are handed out in that order.
 *
 * The children inherit the subdomain of their parent, and both halves of a split boundary side
 * carry the boundary ids the whole side carried, so sidesets and subdomain interfaces come through
 * the surgery unchanged. A child lies inside its parent, which makes the parent the exact source of
 * every value the child takes, so the record names the parent as the host of each new node and of
 * each new element.
 *
 * On a replicated mesh every rank holds the whole mesh and performs the same surgery, so the copies
 * stay identical. A rank cannot measure the whole mesh on its own though, because the sizing field
 * is only readable where the rank owns the degree of freedom that carries it, so each rank measures
 * the elements it owns and the selections are gathered before the marking starts. On a distributed
 * mesh a rank may only replace the elements it owns, and splitting an element leaves every one of
 * its edge neighbors for the closure to repair, so an element with a neighbor another rank owns is
 * not split at all: that red is deferred to a later event instead of reaching across the partition
 * seam, the same way cavity growth pins it. A red that a promotion would have forced across the
 * seam is deferred too, and the marking is rebuilt without it. A deferred element is still
 * oversized at the next event and is selected again, so the deferral only delays the refinement,
 * for as long as the seam stays where it is.
 *
 * This first version is restricted to 2D meshes of TRI3 elements.
 */
class TriSplitRemesher : public Remesher
{
public:
  static InputParameters validParams();

  TriSplitRemesher(const InputParameters & parameters);

  virtual RemeshRecord remesh() override;

private:
  /// The two node ids of an edge in increasing order, so that the elements sharing it agree on it
  using EdgeKey = std::pair<dof_id_type, dof_id_type>;

  /**
   * How one remesh event splits the mesh, as marks on the elements it replaces.
   *
   * The marks are held this way, rather than as nodes and elements, because the closure can still
   * promote an element after it has been visited, and because a red the closure cannot keep on this
   * rank is dropped and the whole pattern rebuilt without it.
   */
  struct RefinementPattern
  {
    /// Ids of the elements split into four at the midpoints of all three of their edges
    std::set<dof_id_type> red;
    /// Ids of the elements bisected into two through the midpoint of their one split edge
    std::set<dof_id_type> green;
    /// Every edge that acquires a midpoint
    std::set<EdgeKey> split_edges;
  };

  /// One vertex of a child triangle, as the node it uses and where that node sits in the parent
  struct ChildVertex
  {
    /// Node of the reference mesh, either a vertex of the parent or a new edge midpoint
    Node * node = nullptr;
    /// Reference (master element) coordinate of that node within the parent element
    Point xi;
  };

  /// The three vertices of one child triangle, in counter-clockwise order
  using ChildTriangle = std::array<ChildVertex, 3>;

  /// The key of side \p side of \p elem
  static EdgeKey edgeKey(const Elem & elem, unsigned int side);

  /// The number of sides of \p elem that \p split_edges gives a midpoint
  static unsigned int countSplitSides(const Elem & elem, const std::set<EdgeKey> & split_edges);

  /**
   * Find the triangles whose longest edge is longer than the target size the sizing variable
   * carries on them. Only the elements this rank owns are measured, on any mesh type, because the
   * sizing field is only readable where this rank owns the degree of freedom that carries it. A
   * replicated mesh gathers what every rank measured, because every rank of one goes on to perform
   * the whole surgery.
   *
   * @return the ids of the oversized elements, in increasing order so that every rank of a
   * replicated mesh selects the same elements in the same order
   */
  std::vector<dof_id_type> selectOversizedElements() const;

  /**
   * Whether this rank may split \p elem into four.
   *
   * The split puts a midpoint on each edge of \p elem, which leaves every one of its edge neighbors
   * non-conforming until the closure bisects or splits that neighbor too. A rank may only replace
   * the entities it owns, so an element with a neighbor it does not own, or with a neighbor that is
   * not in its copy of the mesh at all, cannot be split without reaching onto another rank. On a
   * replicated mesh every element may be split by every rank, because every rank holds and replaces
   * all of them, so nothing is ever deferred there.
   */
  bool mayRefine(const Elem & elem) const;

  /**
   * Mark the elements the event replaces, starting from \p seeds and iterating the red-green
   * closure until no element is promoted any more.
   *
   * @param seeds the ids of the oversized elements, in increasing order
   * @param deferred the elements that may not be marked red, added to whenever the closure demands
   * a split this rank may not perform
   * @param pattern filled with the marks, left half built when this returns false
   * @return false when the closure demanded the split of an element this rank may not touch, in
   * which case the reds that demanded it have been added to \p deferred and the caller has to call
   * again with the reduced set
   */
  bool buildRefinementPattern(const std::vector<dof_id_type> & seeds,
                              std::set<dof_id_type> & deferred,
                              RefinementPattern & pattern) const;

  /**
   * Create the midpoint node of every split edge, on the reference mesh and on the displaced mesh
   * when there is one, and record them.
   *
   * The reds are walked in increasing id order and their sides in local order, so the id an edge
   * midpoint gets depends on the mesh alone. An edge two reds share is created once, by the red
   * with the lower id, and every split edge is a side of at least one red.
   *
   * @param pattern the marks of the event
   * @param next_node_id the first free node id, advanced by one per node created
   * @param midpoint_nodes filled with the midpoint node of every split edge
   * @param record the record the new nodes are appended to
   */
  void createMidpoints(const RefinementPattern & pattern,
                       dof_id_type & next_node_id,
                       std::map<EdgeKey, Node *> & midpoint_nodes,
                       RemeshRecord & record) const;

  /**
   * The boundary ids the children of \p elem have to carry, keyed by the sorted node id pair of the
   * side that carries them. A side that is split contributes both of its halves under the ids of
   * the whole side, which is what keeps a sideset that runs along a split boundary complete.
   *
   * The ids are read off \p elem alone rather than off every element the event replaces, so that a
   * sideset which names one side of an interface does not spread to the other side of it.
   *
   * @param elem the element being replaced
   * @param midpoint_nodes the midpoint node of every split edge, which names the halves
   */
  std::map<EdgeKey, std::vector<boundary_id_type>>
  sideBoundaryIds(const Elem & elem, const std::map<EdgeKey, Node *> & midpoint_nodes) const;

  /**
   * The child triangles that replace \p parent.
   *
   * @param parent the element being replaced
   * @param midpoints the midpoint node of each side of \p parent, null where the side is not split
   * @return the four children of a red when all three sides are split, the two children of a green
   * bisection when one side is split
   */
  static std::vector<ChildTriangle> childTriangles(Elem & parent,
                                                   const std::array<Node *, 3> & midpoints);

  /**
   * Create the children of \p parent on the reference mesh, and on the displaced mesh when there is
   * one, and record them.
   *
   * The parent is recorded but left in the mesh, as the Remesher contract requires, because the
   * engine still has to read the old solution through it.
   *
   * @param parent the element being replaced
   * @param midpoint_nodes the midpoint node of every split edge
   * @param next_elem_id the first free element id, advanced by one per element created
   * @param record the record the new and the replaced entities are appended to
   */
  void spliceElement(Elem & parent,
                     const std::map<EdgeKey, Node *> & midpoint_nodes,
                     dof_id_type & next_elem_id,
                     RemeshRecord & record) const;

  /// Target element size field, a CONSTANT MONOMIAL variable read as one value per element
  const MooseVariableFieldBase & _sizing_variable;

  /// Floor the target element size is held at, unset when the sizing variable alone carries it
  const std::optional<Real> _min_element_size;

  /// Largest number of oversized elements one event may select, unset when they are all selected
  const std::optional<unsigned int> _max_splits_per_event;
};
