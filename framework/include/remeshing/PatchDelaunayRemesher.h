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

#include "libmesh/communicator.h"
#include "libmesh/enum_elem_quality.h"

#include <array>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

class MooseVariableFieldBase;

namespace libMesh
{
template <typename Output>
class FunctionBase;
}

/**
 * Remesher that replaces patches of failing triangles with a Delaunay triangulation of the patch.
 *
 * The geometry of a patch is read off the current mesh: its boundary is a set of closed loops of
 * nodes that are already there, so an arbitrarily deformed interface or crack path is conformed to
 * without any analytic description of it. Those boundary nodes are pinned, which is to say the new
 * triangles reuse the very same Node objects, so exterior boundaries, sidesets and subdomain
 * interfaces that run along a patch boundary come through the surgery untouched. Only the interior
 * of the patch is new.
 *
 * The size of the new triangles is inherited from the length of the boundary edges rather than
 * from the area of the elements that are replaced, which makes remeshing the same patch again a
 * fixed point instead of a sequence that drifts finer with every pass. The target is graded edge
 * by edge rather than taken as one area for the whole patch: the boundary is pinned and cannot be
 * subdivided, so a single target that disagreed with a boundary edge would force slivers against
 * it, and a patch whose boundary spans two mesh sizes has no single target that suits both.
 *
 * Supplying a sizing variable replaces that boundary derived target with one read off a target
 * element size field, and turns on a second selection mode that retriangulates the elements which
 * have become small compared to the size that field asks for. The field is read as one target size
 * per element, the way a size based criterion and the splitting remesher read the same field, so
 * that refinement and coarsening cannot pull against each other.
 *
 * Coarsening a patch is only a fixed point if its boundary is out of over refined territory as
 * well. The interior is retriangulated at the target size, but the boundary is pinned and keeps
 * whatever spacing it had, so a patch whose boundary edges are themselves short against the target
 * leaves behind the elements outside them, still over refined, and the slivers the triangulator has
 * to build against them, which the quality metric then selects. A coarsening patch is therefore
 * grown until no boundary edge of it is short, and one that cannot get there is left alone for this
 * event rather than retriangulated into the state that would call it back.
 *
 * Short means the same thing to the boundary as to an element, but not by the same number. An
 * element is measured by its diameter and a boundary by one edge, and a right isosceles triangle
 * carries a shortest edge of its diameter over sqrt(2), so the boundary bound is the element bound
 * scaled by that factor. Measuring an edge against the diameter bound instead makes the shortest
 * edge of a correctly sized element look short, and the patch then grows out of the over refined
 * region and never stops.
 *
 * A patch whose boundary comes out degenerate in the deformed configuration, by pinching, by
 * collapsing or by crossing itself, is rejected for that step rather than handed to the
 * triangulator. Rejection is not an error: the patch is simply left alone, and remesh() reports
 * that it changed nothing when every candidate patch was rejected.
 *
 * This first version is restricted to 2D meshes and retriangulates TRI3 elements only.
 *
 * On a replicated mesh every rank holds the whole mesh, walks it in the same order and performs the
 * same surgery. Without a sizing variable the copies stay identical without any communication,
 * because every test that selects an element is geometry any rank can measure on any element.
 * Reading the sizing field is not such a test: a rank can only read the target on the elements it
 * owns, so the selection is narrowed to those and then gathered. The targets themselves are
 * gathered with it, because cavity growth on a replicated mesh is not restricted to the elements a
 * rank owns and a cavity therefore has to be sized from elements whose target this rank cannot
 * read. Those two gathers are what pay for the narrowing, and neither is dead code on a run of one
 * rank, where they simply gather nothing.
 *
 * On a distributed mesh a rank retriangulates only the elements it owns: cavity growth stops at an
 * element of another rank, which pins the nodes of the partition seam and confines each rank's
 * surgery to its own elements, and leaves every target a cavity needs readable off the solution.
 * The new entities are numbered out of a block of ids the rank carves above the global maximum, and
 * the ids the surgery deleted are handed to every rank afterwards so that the ghost copies of them
 * are dropped, because libMesh requires a topology change to be performed by every rank that holds
 * the entity and not by the owner alone.
 */
class PatchDelaunayRemesher : public Remesher
{
public:
  static InputParameters validParams();

  PatchDelaunayRemesher(const InputParameters & parameters);

  virtual RemeshRecord remesh() override;

private:
  /// One closed boundary loop of a cavity, as nodes of the current mesh in traversal order
  using BoundaryLoop = std::vector<Node *>;

  /**
   * The triangulation of one accepted cavity, described before any mesh entity is created.
   *
   * The triangulation is held this way, rather than as nodes and elements, because the ids the new
   * entities are given depend on how many entities every rank produces, which is only known once
   * every cavity of every rank has been triangulated.
   */
  struct CavityPatch
  {
    /// Ids of the cavity elements this patch replaces
    std::vector<dof_id_type> old_element_ids;
    /// Positions of the interior vertices of the triangulation, which become new nodes
    std::vector<Point> new_points;
    /// Pinned node of each triangulation vertex, null for the vertices that are new_points
    std::vector<Node *> vertex_nodes;
    /// Index into new_points of each triangulation vertex, only set where vertex_nodes is null
    std::vector<std::size_t> vertex_new_point;
    /// The three vertices of each new triangle, as indices into vertex_nodes
    std::vector<std::array<std::size_t, 3>> triangles;
    /// Boundary ids the cavity carried, keyed by the sorted node id pair of the side that had them
    SideBoundaryIds side_boundary_ids;
    /// Cavity nodes no boundary loop uses, which the retriangulation orphans
    std::vector<Node *> orphaned_nodes;
  };

  /**
   * Find the triangles whose quality metric lies outside the accepted bounds, and, when a sizing
   * variable was given, the triangles whose diameter has fallen below 'coarsen_fraction' of the
   * target size on them. The two tests are independent and a triangle that fails either one is
   * selected. On a distributed mesh only the elements this rank owns are measured, because those
   * are the only ones it may replace. A sizing variable narrows the measurement to the owned
   * elements on a replicated mesh too, because the target is only readable on them, and the
   * selection is gathered afterwards so that every rank ends up with the same one. This is also
   * where a replicated mesh gathers the target of every element into the table the cavities are
   * later sized from, which is why it is not a const operation.
   *
   * @return the ids of the failing elements, in increasing order so that every rank of a
   * replicated mesh selects the same elements in the same order
   */
  std::vector<dof_id_type> selectFailingElements();

  /**
   * The target element size of \p elem, wherever it is legible from. A distributed mesh reads it
   * off the solution, because its cavities hold owned elements only. A replicated mesh reads it out
   * of the gathered table, because its cavities hold elements of every rank and only the owner of
   * an element can read the solution on it.
   *
   * @param elem the element to take the target of, which has to be one of the mesh this event
   * selected on
   * @return the target size, or nothing when the sizing variable is not defined on the subdomain of
   * \p elem, which leaves that element no target
   */
  std::optional<Real> targetSize(const Elem & elem) const;

  /**
   * Build the function that gives the triangulator the target area of \p cavity, out of the target
   * size the sizing variable carries on each of its elements.
   *
   * @param cavity the ids of the cavity elements, in increasing order
   * @return the function, or null when no element of \p cavity carries a target, which leaves the
   * cavity to be sized the way it is sized without a sizing variable
   */
  std::unique_ptr<libMesh::FunctionBase<Real>>
  buildSizingAreaFunction(const std::vector<dof_id_type> & cavity) const;

  /**
   * Grow \p seeds by n_layers point-neighbor layers and split the result into the cavities that
   * are retriangulated independently. Cavities that would have overlapped merge here, because the
   * growth is performed on one shared element set.
   *
   * Growth never crosses an element that is not a TRI3, because the triangulator cannot reproduce
   * one. That is a stopping condition rather than grounds for rejecting the cavity: rejecting
   * after the fact would discard a whole connected patch as soon as it touched a quadrilateral.
   * On a distributed mesh growth stops at an element this rank does not own for the same reason,
   * which keeps one rank's surgery off another rank's elements and pins the partition seam nodes.
   *
   * A cavity whose boundary would pass through one node more than once is healed by absorbing
   * every element around that node, which moves the node into the cavity interior. A pinch that
   * healing cannot remove, because the node's star is blocked by a non-TRI3 element, is left in
   * place for extractBoundaryLoops to reject.
   *
   * With a sizing variable the growth continues until no boundary edge of a cavity is itself short
   * against the target size on it, because a cavity is retriangulated at the target size but its
   * boundary is pinned, so a short boundary edge survives the surgery and hands the next event the
   * elements it meant to remove. A cavity that cannot reach that state, because the growth would
   * have to cross a side it may not cross or because it runs out of rounds, is dropped from the
   * result and counted in \p n_deferred rather than retriangulated with a boundary that would
   * re-trigger it.
   *
   * @param seeds the ids of the failing elements
   * @param n_deferred filled with the number of cavities dropped for a boundary that is still
   * short against the target size, zero when there is no sizing variable
   * @return one vector of element ids per cavity, each in increasing order
   */
  std::vector<std::vector<dof_id_type>> buildCavities(const std::vector<dof_id_type> & seeds,
                                                      unsigned int & n_deferred);

  /**
   * Find the sides on the boundary of \p cavity whose edge is short against the target size of the
   * cavity element that carries it, which are the sides a coarsening cavity may not stop at.
   *
   * The test is the one the selection applies to an element, restated for a single edge: the
   * selection compares Elem::hmax() to 'coarsen_fraction' of the target, and this compares one edge
   * to that same bound scaled from a diameter to an edge. The two thresholds are therefore not the
   * same number. Stating the bound in the measure the boundary offers is what keeps a correctly
   * sized element from counting as short and growing the cavity past the over refined region.
   *
   * @param cavity the ids of the cavity elements, in increasing order
   * @return the element id and side of each such boundary side, in increasing order, empty when
   * the boundary of \p cavity is everywhere out of over refined territory
   */
  std::vector<std::pair<dof_id_type, unsigned int>>
  overRefinedBoundarySides(const std::vector<dof_id_type> & cavity) const;

  /**
   * Find the nodes where the boundary of \p cavity passes through more than once. A node on the
   * boundary of a simple polygon carries exactly one outgoing and one incoming boundary edge; a
   * node with more of either is a pinch, and a cavity with a pinch cannot be triangulated as a
   * polygon.
   *
   * @param cavity the ids of the cavity elements
   * @return the ids of the pinch nodes, in increasing order
   */
  std::vector<dof_id_type> findPinchNodes(const std::vector<dof_id_type> & cavity) const;

  /**
   * Extract the closed boundary loops of a cavity, the outer loop first and the loops around the
   * regions the cavity encloses after it.
   *
   * @param cavity the ids of the cavity elements
   * @param loops filled with the loops, each a list of nodes in traversal order
   * @return false when a node carries more than one outgoing boundary edge, which is a pinch that
   * makes the loop through that node ambiguous and rejects the cavity
   */
  bool extractBoundaryLoops(const std::vector<dof_id_type> & cavity,
                            std::vector<BoundaryLoop> & loops) const;

  /**
   * Coarsen the external boundary stretches of \p loops by dropping the nodes that
   * 'coarsen_boundary_fraction' selects: a node whose two loop edges are both external boundary
   * sides carrying the same sideset ids, both shorter than that fraction of the target edge
   * length, and whose removal leaves the loop geometrically unchanged because it sits on the
   * straight line between its neighbors. The merged edge may not itself exceed the target.
   *
   * A dropped node leaves every loop, so collectOrphanedNodes picks it up and the surgery deletes
   * it - which is safe, because its two external loop edges bound its whole element fan inside
   * the cavity. This is the one place the remesher un-pins a boundary node; a partition seam and
   * every interior loop edge stay pinned.
   *
   * Does nothing unless 'coarsen_boundary_fraction' is positive.
   *
   * @param cavity the ids of the cavity elements
   * @param loops the boundary loops of the cavity, thinned in place
   * @param merged_side_ids filled with the sideset ids of each merged edge, keyed by its sorted
   * node pair, because no old element side spans those nodes for triangulateCavity to read them
   * off
   */
  void thinBoundaryLoops(const std::vector<dof_id_type> & cavity,
                         std::vector<BoundaryLoop> & loops,
                         SideBoundaryIds & merged_side_ids) const;

  /**
   * Check that \p loops describe a simple polygon with holes in the current configuration.
   *
   * @param loops the boundary loops of the cavity, the outer loop first
   * @param mean_edge_length filled with the mean length of the boundary edges, the length the
   * geometric tolerances of the cavity are taken relative to
   * @return false when a loop is too short, has a collapsed edge, is wound the wrong way, or when
   * two boundary edges that are not neighbors touch or cross
   */
  bool validateBoundaryLoops(const std::vector<BoundaryLoop> & loops,
                             Real & mean_edge_length) const;

  /**
   * Triangulate the region bounded by \p loops and map the triangulated boundary vertices back
   * onto the nodes of \p loops, so that only genuinely interior vertices become new nodes.
   *
   * @param cavity the ids of the cavity elements
   * @param loops the boundary loops of the cavity, the outer loop first
   * @param mean_edge_length the mean length of the boundary edges
   * @param patch filled with the triangulation
   * @return false when the triangulation did not reproduce every pinned node, or produced an
   * element that is not a counter-clockwise TRI3
   */
  bool triangulateCavity(const std::vector<dof_id_type> & cavity,
                         const std::vector<BoundaryLoop> & loops,
                         Real mean_edge_length,
                         CavityPatch & patch) const;

  /**
   * @param cavity the ids of the cavity elements
   * @param loops the boundary loops of the cavity
   * @return the cavity nodes that no boundary loop uses, which no element will use once the cavity
   * elements are gone
   */
  std::vector<Node *> collectOrphanedNodes(const std::vector<dof_id_type> & cavity,
                                           const std::vector<BoundaryLoop> & loops) const;

  /**
   * Create the nodes and elements of \p patch on the reference mesh, and on the displaced mesh
   * when there is one, and record them.
   *
   * The cavity elements and the orphaned nodes are recorded but left in the mesh, as the Remesher
   * contract requires, because the engine still has to read the old solution through them.
   *
   * @param patch the triangulation to create
   * @param next_node_id the first free node id, advanced by one per node created
   * @param next_elem_id the first free element id, advanced by one per element created
   * @param record the record the new and the replaced entities are appended to
   */
  void splicePatch(const CavityPatch & patch,
                   dof_id_type & next_node_id,
                   dof_id_type & next_elem_id,
                   RemeshRecord & record) const;

  /// Number of point-neighbor layers grown around each failing element to form a cavity
  const unsigned int _n_layers;

  /**
   * One target area for the whole patch. Zero leaves the target to the sizing variable, and to the
   * cavity boundary edge length scale when there is no sizing variable.
   */
  const Real _desired_area;

  /**
   * Target element size field, a CONSTANT MONOMIAL variable read as one value per element, null
   * when none was given. Null is what turns off both the size selection and the sizing of the new
   * triangles off the field, leaving the quality selection and the boundary derived target alone.
   */
  const MooseVariableFieldBase * const _sizing_variable;

  /**
   * Fraction of the target size below which an element counts as over refined. A cavity boundary
   * edge counts as over refined against the same fraction scaled from a diameter to an edge, so one
   * number says what coarsening removes and how far it has to reach to do it, each in the measure
   * that test can see rather than as one shared threshold. Only read when a sizing variable was
   * given, which is the only case the parameters accept it in.
   */
  const Real _coarsen_fraction;

  /**
   * Fraction of the target edge length below which an external boundary edge counts as crowded,
   * which is what lets thinBoundaryLoops drop the collinear boundary nodes between two such
   * edges. Zero keeps every boundary node pinned.
   */
  const Real _coarsen_boundary_fraction;

  /**
   * Target element size of every element that carries one, keyed by element id, as gathered from
   * every rank. It is what lets a replicated mesh size a cavity out of elements whose target this
   * rank cannot read off the solution.
   *
   * selectFailingElements resets it and gathers it again at the start of every remesh event,
   * because the ids in it name the elements of the mesh as it stood then and the surgery of that
   * event replaces them. Unset means no event has gathered it yet, which is also what a distributed
   * mesh leaves it as: there the target is read straight off the solution.
   */
  std::optional<std::map<dof_id_type, Real>> _gathered_target_sizes;

  /// Quality metric that decides which triangles fail
  const libMesh::ElemQuality _quality_metric;

  /// Lower bound of the quality metric, below which a triangle fails
  Real _lower_bound;

  /// Upper bound of the quality metric, above which a triangle fails
  Real _upper_bound;

  /**
   * The communicator each rank triangulates on.
   *
   * The triangulator serializes and reduces over the communicator of the boundary mesh it is
   * given. Every rank triangulates every cavity redundantly, so splitting one singleton
   * communicator per rank turns all of those collectives into rank local work rather than
   * repeating the same reduction across the whole run.
   */
  libMesh::Parallel::Communicator _cavity_comm;
};
