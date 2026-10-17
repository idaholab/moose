//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "Remesher.h"

#include "libmesh/parallel_object.h"
#include "libmesh/point.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

class FEProblemBase;
class MooseMesh;
class SystemBase;

namespace libMesh
{
class DofMap;
class DofObject;
class Elem;
class System;
template <typename T>
class NumericVector;
}

/**
 * Carries every stored solution state of every system across a mesh replacement.
 *
 * This is an engine utility rather than a MooseObject: the Remeshing engine owns one and drives it
 * around FEProblemBase::meshChanged(). The transfer is two phased because the two halves of the
 * work live on opposite sides of that call:
 *
 * - gather() runs while the replaced elements are still in the mesh. That is the only time the
 *   record's source points, which are an old element and a reference coordinate inside it, can be
 *   evaluated, and the only time the old degree of freedom numbering is still in place.
 * - scatter() runs after meshChanged() has rebuilt the equation systems, which is the first time
 *   the new entities have degrees of freedom to write into.
 *
 * The engine calls meshChanged() with contract_mesh = true, so MeshBase::contract() and
 * DofMap::distribute_dofs() renumber the degrees of freedom of the entities that survive the
 * surgery as well. The gathered values are therefore keyed on the entity that carries them rather
 * than on a degree of freedom index, and scatter() restores the surviving entities in addition to
 * filling the new ones.
 *
 * A new entity takes the value its source point has on the old mesh,
 *
 *     u_new(p) = sum_j u_old_j * phi_j(xi(p))
 *
 * where phi_j are the shape functions of the old element that hosts p. The record locates one
 * source point per new node and one per new element, so a variable is only carried when it holds a
 * single degree of freedom per mesh entity, which covers first order LAGRANGE and CONSTANT
 * MONOMIAL.
 *
 * Stateful material properties are not transferred here. They ride across the mesh replacement as
 * the auxiliary variables of the [ProjectedStatefulMaterialStorage] block, which are ordinary
 * variables to this class; checkStatefulCoverage() fails the setup when a stateful property is not
 * covered that way.
 */
class RemeshTransfer : public libMesh::ParallelObject
{
public:
  RemeshTransfer(FEProblemBase & problem);

  /**
   * Error out when a stateful material property of the problem would lose its state at a remesh.
   *
   * Remeshing has no path for the raw quadrature point data of MaterialPropertyStorage, which is
   * keyed on the elements the surgery deletes and which FEProblemBase::initElementStatefulProps()
   * re-seeds from scratch on the new elements. The only path across a mesh replacement is the
   * [ProjectedStatefulMaterialStorage] block, whose auxiliary variables this class carries like
   * any other variable.
   *
   * A property only becomes stateful when an object requests a state past the current one, since
   * a declaration is always made at the current state, and use_interpolated_state rewrites such a
   * request into a current state request on the property InterpolatedStatefulMaterial
   * reconstitutes from those auxiliary variables. A property that is both projected and consumed
   * through the projection is therefore not stateful at all, which makes any property that is
   * still stateful here one whose old state some object reads off the raw storage. The auxiliary
   * variable of the projection only separates the two remedies: set the block up, or set
   * use_interpolated_state on the objects that read the old state.
   *
   * Called once from Remeshing::initialSetup(), so that a simulation that would silently lose
   * state fails before its first solve.
   */
  void checkStatefulCoverage() const;

  /**
   * Read the old solution out of the mesh.
   *
   * Called by the engine while the replaced elements, and the degrees of freedom numbered on them,
   * are still in the mesh. Snapshots the degrees of freedom of the surviving entities this rank
   * owns, and evaluates the record's source points for the new entities.
   *
   * @param record the record of the surgery the remesher just performed
   */
  void gather(const Remesher::RemeshRecord & record);

  /**
   * Write the gathered solution back into the rebuilt equation systems.
   *
   * Called by the engine after FEProblemBase::meshChanged() has redistributed the degrees of
   * freedom. Restores the surviving entities from the snapshot, fills the new entities from the
   * evaluated source points, and closes every vector it wrote.
   *
   * @param record the record of the surgery the remesher just performed
   */
  void scatter(const Remesher::RemeshRecord & record);

private:
  /// A degree of freedom named by the entity that carries it rather than by its index, which the
  /// redistribution changes. The entity is held by address because MeshBase::contract() renumbers
  /// the ids of the surviving nodes and elements as well, while it leaves the objects in place
  struct EntityDof
  {
    /// The node or element the degree of freedom hangs off
    const libMesh::DofObject * dof_object;
    /// The variable of the system
    unsigned int var;
    /// The component of the variable on this entity
    unsigned int comp;
  };

  /// Everything gather() read out of one system
  struct SystemSnapshot
  {
    /// The degrees of freedom of the surviving entities this rank owns
    std::vector<EntityDof> entity_dofs;
    /// The scalar variable degrees of freedom this rank owns, as (variable, index in its block)
    std::vector<std::pair<unsigned int, unsigned int>> scalar_dofs;
    /// Per vector of the system, the value of every entry of entity_dofs
    std::vector<std::vector<Real>> entity_values;
    /// Per vector of the system, the value of every entry of scalar_dofs
    std::vector<std::vector<Real>> scalar_values;
  };

  /// A request for the values at one source point, as the reference coordinate of the point and
  /// the id of the old element that hosts it
  using SourceQuery = std::pair<Point, dof_id_type>;

  /// A copy of every vector of every system that this rank can read everywhere it needs to,
  /// indexed by system and then by vector
  using ReadVectors = std::vector<std::vector<std::unique_ptr<libMesh::NumericVector<Number>>>>;

  /**
   * The systems whose state is carried across a mesh replacement: the solver systems followed by
   * the auxiliary system.
   */
  std::vector<SystemBase *> systems() const;

  /**
   * Every vector of \p system that its degree of freedom map numbers: the current solution
   * followed by the registered vectors, in the order of the names they are registered under. MOOSE
   * keeps the older solution states, the previous Newton iterate and the time integrator work
   * vectors as registered vectors, so this is the complete list of the states the system stores.
   */
  static std::vector<libMesh::NumericVector<Number> *> systemVectors(libMesh::System & system);

  /**
   * A copy of \p vector that this rank can read at every degree of freedom of its local elements.
   * A PARALLEL vector does not hold the degrees of freedom of a local element that its neighbors
   * own, which the interpolation off the old mesh needs.
   */
  static std::unique_ptr<libMesh::NumericVector<Number>>
  localizedCopy(const libMesh::NumericVector<Number> & vector, const libMesh::DofMap & dof_map);

  /// The number of values a source point carries, one per system, variable and vector
  std::size_t sourceValuesPerPoint() const;

  /**
   * Record the value of every degree of freedom the surviving entities this rank owns carry.
   *
   * @param record the record of the surgery, whose entities are the ones that are skipped
   * @param systems the systems to snapshot
   * @param reads the readable copies of the vectors of \p systems
   */
  void snapshotSurvivingEntities(const Remesher::RemeshRecord & record,
                                 const std::vector<SystemBase *> & systems,
                                 const ReadVectors & reads);

  /**
   * Evaluate the old mesh at the source point of every new entity of the record.
   *
   * A source point is answered by the rank that owns the old element it sits in, which is the only
   * rank that holds all of the degrees of freedom of that element. The exchange is collective, so
   * every rank takes part in it, including the ranks whose surgery produced nothing.
   *
   * @param record the record of the surgery, which supplies the source points
   * @param systems the systems to evaluate
   * @param reads the readable copies of the vectors of \p systems
   */
  void evaluateSourcePoints(const Remesher::RemeshRecord & record,
                            const std::vector<SystemBase *> & systems,
                            const ReadVectors & reads);

  /**
   * Interpolate the value of every system, variable and vector at one source point of the old mesh.
   *
   * @param old_elem the old element that hosts the point
   * @param xi the reference coordinate of the point within \p old_elem
   * @param systems the systems to evaluate
   * @param reads the readable copies of the vectors of \p systems
   * @param values filled with one value per system, variable and vector
   */
  void interpolate(const libMesh::Elem & old_elem,
                   const Point & xi,
                   const std::vector<SystemBase *> & systems,
                   const ReadVectors & reads,
                   std::vector<Real> & values) const;

  /**
   * Give back to the surviving entities the values gather() read off them, at the degree of
   * freedom indices they are numbered at now.
   *
   * @param systems the systems to restore, in the order gather() snapshotted them
   */
  void restoreSurvivingEntities(const std::vector<SystemBase *> & systems);

  /**
   * Give the new entities of the record the values their source points carry.
   *
   * @param record the record of the surgery, which supplies the new entities
   * @param systems the systems to fill, in the order gather() evaluated them
   */
  void fillNewEntities(const Remesher::RemeshRecord & record,
                       const std::vector<SystemBase *> & systems);

  /**
   * Write one interpolated value into the degree of freedom \p dof_object carries for \p var.
   *
   * @param dof_object the new node or new element that receives the value
   * @param system the system of \p var, used to name it in an error
   * @param var the variable of the system
   * @param value the interpolated value
   * @param vector the vector to write into
   */
  void setNewEntityDof(const libMesh::DofObject & dof_object,
                       const libMesh::System & system,
                       unsigned int var,
                       Real value,
                       libMesh::NumericVector<Number> & vector) const;

  /// The problem being remeshed
  FEProblemBase & _fe_problem;

  /// The reference mesh the surgery is performed on
  MooseMesh & _mesh;

  /// What gather() read out of each system, in the order systems() returns them
  std::vector<SystemSnapshot> _snapshots;

  /// The interpolated old mesh values of each new node of the record
  std::vector<std::vector<Real>> _new_node_values;

  /// The interpolated old mesh values of each new element of the record
  std::vector<std::vector<Real>> _new_element_values;
};
