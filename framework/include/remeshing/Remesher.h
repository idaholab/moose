//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseTypes.h"
#include "SetupInterface.h"

#include "libmesh/point.h"

#include <array>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class FEProblemBase;
class MooseMesh;
class MooseVariableFieldBase;

namespace libMesh
{
class MeshBase;
}

/**
 * Base class for the objects that replace elements of the reference mesh.
 *
 * A Remesher is driven by the Remeshing engine, which calls remesh() when a RemeshCriterion fires.
 * remesh() adds the new nodes and elements to the reference mesh and returns a RemeshRecord that
 * describes them and says where their values come from on the old mesh.
 *
 * The remesher must leave the entities it replaces in the mesh. The engine deletes them, and
 * erases their stateful material property storage, only after RemeshTransfer has read the old
 * solution through the record. A remesher that frees an element itself leaves RemeshTransfer
 * reading a dangling pointer.
 */
class Remesher : public MooseObject, public SetupInterface
{
public:
  static InputParameters validParams();

  Remesher(const InputParameters & parameters);

  /**
   * A point on the old mesh that supplies the values of one new entity.
   */
  struct RemeshSourcePoint
  {
    /// Old element containing the point, valid until the engine deletes the replaced elements
    const Elem * old_elem = nullptr;
    /// Reference (master element) coordinate of the point within old_elem
    Point xi;
  };

  /**
   * Everything one remesh event produced, and everywhere the new values come from.
   *
   * new_node_sources is parallel to new_nodes, and new_element_sources is parallel to
   * new_elements: entry i of a source vector is the old mesh point that supplies entry i of the
   * matching entity vector. Every source point has to be located while the old elements are still
   * in the mesh, which means inside remesh().
   */
  struct RemeshRecord
  {
    /// Whether this rank changed the mesh, false when every candidate cavity was rejected
    bool changed = false;
    /// Nodes the surgery added to the reference mesh
    std::vector<Node *> new_nodes;
    /// Elements the surgery added to the reference mesh
    std::vector<Elem *> new_elements;
    /// Nodes the surgery orphaned, still in the mesh, that the engine deletes
    std::vector<Node *> replaced_nodes;
    /// Elements the surgery replaced, still in the mesh, that the engine deletes
    std::vector<Elem *> replaced_elements;
    /// Old mesh source point of each entry of new_nodes
    std::vector<RemeshSourcePoint> new_node_sources;
    /// Old mesh source point of each entry of new_elements
    std::vector<RemeshSourcePoint> new_element_sources;
  };

  /**
   * Replace elements of the reference mesh with new ones.
   *
   * The surgery operates on the reference coordinates x = X0 + d, never on the displaced
   * coordinates: with true displacements the displacement variables are interpolated onto the new
   * mesh like any other variable, which keeps the description total Lagrangian.
   *
   * On a distributed mesh a remesher may only add and replace the entities its own rank owns. The
   * engine relies on it: it hands the ids every rank deleted to every other rank so that the ghost
   * copies of them are dropped, which only restores a valid mesh if no rank touched an entity
   * another rank owns.
   *
   * @return the record of the new entities and of the old mesh points that supply their values
   */
  virtual RemeshRecord remesh() = 0;

protected:
  /// The boundary ids a side carries, keyed by the sorted node id pair of that side
  using SideBoundaryIds =
      std::map<std::pair<dof_id_type, dof_id_type>, std::vector<boundary_id_type>>;

  /**
   * The two node ids of a side in increasing order, so that the elements sharing it agree on the
   * key. It lives on the base because every remesher that keys a side by its nodes has to order the
   * pair the same way.
   */
  static std::pair<dof_id_type, dof_id_type> sortedNodePair(dof_id_type first, dof_id_type second);

  /**
   * Check that \p variable is a CONSTANT MONOMIAL variable, which is what lets a target element
   * size be read as the single degree of freedom an element carries for it. It lives on the base
   * because every remesher that reads a sizing field reads it that way.
   *
   * @param param the parameter the variable was named by, which the error is attributed to
   * @param variable the sizing variable
   */
  void checkElementalSizingVariable(const std::string & param,
                                    const MooseVariableFieldBase & variable) const;

  /**
   * Read the target element size \p variable carries on \p elem off the solution, as the single
   * degree of freedom a CONSTANT MONOMIAL variable holds for the element rather than interpolated.
   *
   * @param variable the sizing variable
   * @param elem the element to read the target on, which this rank has to own: the solution is
   * readable at the owned degrees of freedom plus the send list, and the degrees of freedom
   * partition by processor id even where the elements do not
   * @param size_floor the floor the target is held at; without one a target that is not positive
   * is an error
   * @return the target size, or nothing when \p variable is not defined on the subdomain of
   * \p elem, which leaves that element no target
   */
  std::optional<Real> readTargetSize(const MooseVariableFieldBase & variable,
                                     const Elem & elem,
                                     std::optional<Real> size_floor) const;

  /**
   * Gather the targets every rank read on the elements it owns into one table keyed by element id,
   * so that a rank of a replicated mesh can size the elements it does not own. It gathers nothing
   * on a run of one rank, which owns every element, rather than being dead code there.
   *
   * @param target_ids the ids of the owned elements that carry a target
   * @param targets the target of each entry of \p target_ids
   * @return the table, holding the readings of every rank
   */
  std::map<dof_id_type, Real> gatherTargetSizes(std::vector<dof_id_type> target_ids,
                                                std::vector<Real> targets) const;

  /**
   * Find where on the old mesh \p p takes its values from, among the elements of
   * \p candidate_ids. A point that misses every containment test by round-off, because it sits on
   * the side of a candidate, takes the candidate whose centroid is nearest.
   *
   * @param candidate_ids the ids of the old elements that may contain \p p, which still have to be
   * in the mesh
   * @param p a point inside the union of the candidates, in the current configuration
   * @return the first candidate, in the order of \p candidate_ids, that contains \p p and the
   * reference coordinate of \p p in it
   */
  RemeshSourcePoint locateSourcePoint(const std::vector<dof_id_type> & candidate_ids,
                                      const Point & p) const;

  /**
   * The displaced mesh the new entities are mirrored onto, null when the problem has no
   * displacements. It is a mesh of its own rather than a view of the reference mesh, so a topology
   * change has to be performed on it separately, and matching ids are what pairs the two up again.
   */
  libMesh::MeshBase * displacedMesh() const;

  /**
   * Choose the first id the nodes and the elements this remesh event creates are numbered from.
   *
   * On a replicated mesh every rank creates the same entities and numbers them identically. On a
   * distributed mesh the ranks create different entities, so each takes a block of ids above the
   * global maximum, sized by what it is about to create and placed after the blocks of the lower
   * ranks. The blocks are disjoint by construction, which is what lets a rank number its entities
   * without asking any other rank which ids it used.
   *
   * It is collective on a distributed mesh, so every rank has to reach it, including one that
   * creates nothing at all.
   *
   * @param n_new_nodes the number of nodes this rank is about to create
   * @param n_new_elements the number of elements this rank is about to create
   * @param first_node_id filled with the first node id this rank may use
   * @param first_elem_id filled with the first element id this rank may use
   */
  void reserveNewEntityIds(dof_id_type n_new_nodes,
                           dof_id_type n_new_elements,
                           dof_id_type & first_node_id,
                           dof_id_type & first_elem_id) const;

  /**
   * Add one node to the reference mesh, mirror it onto the displaced mesh when there is one, and
   * record it. The node carries the partition and the system count of the element it takes its
   * values from.
   *
   * It lives on the base because every remesher that adds a node has to mirror it under the same
   * id, which is the only thing that pairs the two meshes up again.
   *
   * @param point the position of the node on the reference mesh
   * @param source the old mesh point that supplies the values of the node
   * @param next_node_id the id the node is given, advanced by one
   * @param record the record the new node is appended to
   * @return the added node of the reference mesh
   */
  Node * addNode(const Point & point,
                 const RemeshSourcePoint & source,
                 dof_id_type & next_node_id,
                 RemeshRecord & record) const;

  /**
   * Add one triangle to \p target, with the subdomain and the partition of \p source and the
   * boundary ids its sides carry. It lives on the base because every remesher that replaces
   * triangles has to build them, and mirror them onto the displaced mesh, the same way.
   *
   * @param target the mesh the triangle is added to, the reference mesh or the displaced mesh
   * @param nodes the three nodes of the triangle, in counter-clockwise order
   * @param id the element id, which is the same on the reference and on the displaced mesh
   * @param source the element of \p target the triangle takes its subdomain and partition from
   * @param side_boundary_ids the boundary ids the new triangles have to carry
   * @return the added element
   */
  Elem * addTriangle(libMesh::MeshBase & target,
                     const std::array<Node *, 3> & nodes,
                     dof_id_type id,
                     const Elem & source,
                     const SideBoundaryIds & side_boundary_ids) const;

  /**
   * Add one triangle to the reference mesh, mirror it onto the displaced mesh when there is one,
   * and record it. The triangle takes its subdomain and partition from the element of \p source,
   * and the mirrored one from the displaced copy of that element.
   *
   * It lives on the base for the reason addNode does: the mirrored triangle has to carry the same
   * id, which is the only thing that pairs the two meshes up again.
   *
   * @param nodes the three nodes of the triangle on the reference mesh, in counter-clockwise order
   * @param source the old mesh point that supplies the values of the triangle
   * @param side_boundary_ids the boundary ids the new triangle has to carry
   * @param next_elem_id the id the triangle is given, advanced by one
   * @param record the record the new triangle is appended to
   * @return the added element of the reference mesh
   */
  Elem * addMirroredTriangle(const std::array<Node *, 3> & nodes,
                             const RemeshSourcePoint & source,
                             const SideBoundaryIds & side_boundary_ids,
                             dof_id_type & next_elem_id,
                             RemeshRecord & record) const;

  /// The problem being remeshed
  FEProblemBase & _fe_problem;

  /// The reference mesh the surgery is performed on
  MooseMesh & _mesh;
};

inline std::pair<dof_id_type, dof_id_type>
Remesher::sortedNodePair(const dof_id_type first, const dof_id_type second)
{
  return first < second ? std::make_pair(first, second) : std::make_pair(second, first);
}
