//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConsoleStreamInterface.h"
#include "MooseTypes.h"
#include "PerfGraphInterface.h"
#include "Remesher.h"
#include "Restartable.h"

#include "libmesh/parallel_object.h"
#include "libmesh/point.h"

#include <map>
#include <memory>
#include <vector>

class DisplacedProblem;
class FEProblemBase;
class MeshSmootherBase;
class MooseMesh;
class RemeshCriterion;
class RemeshTransfer;
class TransientBase;

/**
 * Takes care of everything related to remeshing.
 *
 * The engine is owned by FEProblemBase and driven by the executioner, exactly the way Adaptivity
 * is. Its arbitrary Lagrangian Eulerian bookkeeping is total since the last remesh: X0 is the
 * reference node coordinates snapshotted at the last remesh, d is the pseudo-displacement
 * accumulated since then, and on every attempt at a time step the reference coordinates are set to
 *
 *     x = X0 + d
 *
 * At a remesh event X0 becomes x and d goes back to zero, because d is bookkeeping rather than a
 * solved physical field. The coordinates are never accumulated onto in place, which is what makes
 * the reset exact.
 *
 * The executioner repeats a time step whose solve failed, so the engine is entered more than once
 * for the same step while the mesh still has to end up where a step solved on the first attempt
 * would have left it. d as it stood at the start of the attempt is therefore kept, and a repeat
 * puts it back before moving the mesh again rather than advancing d a second time. See
 * remeshingStep() for the one attempt that cannot be re-entered that way.
 *
 * The displaced mesh needs no special handling: DisplacedProblem holds a reference to this same
 * MooseMesh and recomputes the displaced coordinates from the reference coordinates plus the
 * displacement solution on every DisplacedProblem::updateMesh().
 */
class Remeshing : public ConsoleStreamInterface,
                  public PerfGraphInterface,
                  public libMesh::ParallelObject,
                  public Restartable
{
public:
  /// A Point per node id, over the nodes of the reference mesh this processor holds, the ghosted
  /// ones as well as the local ones
  using PointMap = std::map<dof_id_type, Point>;

  Remeshing(FEProblemBase & fe_problem);
  virtual ~Remeshing();

  /**
   * Initialize and turn on remeshing for the simulation. Called by RemeshingAction, the way
   * AdaptivityAction calls Adaptivity::init().
   *
   * @param mesh_movement whether a MeshSmootherBase moves the mesh between remesh events
   * @param displacements the displacement variables of the problem, empty when it has none
   * @param check_interval the number of time steps between two evaluations of the criteria
   * @param initial_remesh_cycles the largest number of remesh cycles performed on the initial
   * condition before the transient starts, zero for none
   */
  void init(bool mesh_movement,
            const std::vector<VariableName> & displacements,
            unsigned int check_interval,
            unsigned int initial_remesh_cycles);

  /**
   * Is remeshing on?
   *
   * @return true if init() has been called, otherwise false
   */
  bool isOn() const { return _on; }

  /**
   * Does a smoother move the mesh between remesh events? When this is false the pseudo-displacement
   * d is identically zero, so anything that measures d requires it to be true.
   */
  bool meshMovementEnabled() const { return _mesh_movement; }

  /**
   * The displacement variables of the problem, empty when the problem has none.
   */
  const std::vector<VariableName> & displacements() const { return _displacements; }

  /**
   * The number of remesh events performed so far.
   */
  unsigned int remeshCount() const { return _remesh_count; }

  /**
   * The total pseudo-displacement d accumulated since the last remesh, keyed by node id. The
   * smoother writes it through the private mutable overload below and everything else reads it;
   * it is zero unless meshMovementEnabled(). It carries the same nodes as referenceCoordinates().
   */
  const PointMap & pseudoDisplacement() const { return _pseudo_displacement; }

  /**
   * The reference node coordinates X0 snapshotted at the last remesh, keyed by node id.
   *
   * The snapshot covers every node this processor held when it was taken, the ghosted ones as well
   * as the local ones, and the ghost layer is not fixed for the lifetime of a snapshot. The entries
   * of the nodes this processor does not own are therefore refreshed from their owners at the start
   * of every step rather than only at a remesh event.
   */
  const PointMap & referenceCoordinates() const { return _reference_coordinates; }

  ///@{
  /**
   * Take ownership of an object built by the RemeshingAction.
   */
  void addRemesher(std::shared_ptr<Remesher> remesher);
  void addCriterion(std::shared_ptr<RemeshCriterion> criterion);
  void addSmoother(std::shared_ptr<MeshSmootherBase> smoother);
  ///@}

  /**
   * Check the setup, snapshot X0, and then set up the criteria, the remeshers and the smoother, in
   * that order, so that an object that sizes itself from X0 sees it populated.
   *
   * Called at the end of FEProblemBase::initialSetup(), which is late enough that restarted data
   * has been restored and that [Adaptivity] has been set up.
   */
  void initialSetup();

  /**
   * Move the mesh, and remesh it when a criterion fires.
   *
   * Called from TransientBase::takeStep(), after the time step size is computed and before the
   * solve, and so once per attempt at a time step rather than once per time step: the executioner
   * repeats a step whose solve failed and this runs again. The mesh is still moved only once per
   * step. A fresh attempt records the d it starts from, and a repeat restores that d and the
   * coordinates x = X0 + d it produces before moving the mesh with the time step size the repeat
   * was handed, so the motion always matches the span of time the step is finally solved over.
   *
   * A repeat is recognized by the executioner reporting that the previous solve failed. XFEM
   * repeats a step while reporting a converged solve, which this cannot tell apart from a fresh
   * step, so initialSetup() refuses that combination rather than advancing d twice in silence.
   *
   * The one attempt that cannot be re-entered that way is one in which a remesh event fired. The
   * surgery replaced X0 and reissued the node ids d is keyed by, and the nodes it created exist
   * only on the mesh that attempt had already moved, so the configuration the step started from is
   * gone. Such a repeat keeps the moved and replaced mesh instead, which reproduces the failed
   * attempt exactly when the repeat solves the same span of time, and is refused when the time step
   * size changed.
   *
   * @param dt the time step size the executioner is about to solve with
   */
  void remeshingStep(Real dt);

  /**
   * Set the reference coordinate of every node of the reference mesh to x = X0 + d.
   *
   * The ghosted nodes are positioned along with the local ones. libMesh requires the mesh geometry
   * to be parallel consistent, and a ghosted coordinate left behind reaches the displaced mesh,
   * which is recomputed over every node it holds, the geometric decisions the next remesh makes on
   * the ghosted elements, and the next snapshot of X0, which would then carry it for good.
   */
  void applyPseudoDisplacement();

private:
  /// The smoother is the only object that writes the pseudo-displacement
  friend class MeshSmootherBase;

  /// The mutable pseudo-displacement, reachable only by the engine and the smoother
  PointMap & pseudoDisplacement() { return _pseudo_displacement; }

  /// Set X0 to the current reference coordinates and d to zero, over every node this processor
  /// holds at the time of the call
  void snapshotReferenceCoordinates();

  /**
   * Fail when the node ids no longer name the nodes they named when X0 and d were snapshotted.
   *
   * Both maps are keyed by node id, which requires the ids to be stable between two remesh events.
   * A renumbering reports itself through FEProblemBase::meshChanged() alone, which the engine does
   * not yet rebuild its bookkeeping on, and a renumbered id that collides with the id of another
   * node hands that node's reference coordinate to this one without any lookup failing. So the
   * keys are tested rather than trusted: every node that still carries a key has to sit where the
   * sum x = X0 + d filed under it says it does.
   *
   * The one renumbering known to reach a distributed mesh, the one Exodus output performs, is
   * refused in initialSetup(). This stays as the backstop for any other source, which would
   * otherwise have no detector at all.
   *
   * Runs before syncGhostedBookkeeping(), because a renumbering also drops keys and would
   * otherwise be reported as a gap in the ghost coverage.
   */
  void checkBookkeepingKeys() const;

  /**
   * Fill X0 and d for the nodes this processor holds but did not hold when they were snapshotted,
   * by pulling the values from the processors that own those nodes.
   *
   * Serializing a distributed mesh and releasing it again, which any Exodus output of one does,
   * re-prunes the ghost layer behind the engine's back and can leave this processor holding nodes
   * that neither map has a key for. Both position-application loops, applyPseudoDisplacement() and
   * the assembly of a smoother, visit the ghosted nodes as well as the local ones, so the gap is
   * closed rather than skipped: skipping it would trade a map lookup failure for a ghosted node
   * that silently stops following its owner.
   *
   * Only a node this processor does not own can be missing, because a processor always holds the
   * nodes it owns and the snapshot covers every node it holds. A missing owned node is therefore a
   * renumbering that checkBookkeepingKeys() did not catch, and it is reported rather than filled.
   */
  void syncGhostedBookkeeping();

  /**
   * Record the state a fresh attempt at a time step starts from, so that a repeat of that attempt
   * can be put back to it.
   *
   * None of what it records is restartable, because a checkpoint is only written at the end of a
   * converged step and a restored run therefore always resumes on a fresh attempt.
   *
   * @param dt the time step size the attempt is about to move the mesh with
   */
  void beginStepAttempt(Real dt);

  /**
   * Put the engine back to the state the failed attempt at this step started from.
   *
   * @param dt the time step size the repeat is about to solve with
   * @return whether the mesh still has to be moved and the criteria still have to be evaluated,
   *         which is false when the failed attempt already remeshed this step
   */
  bool reenterStepAttempt(Real dt);

  /**
   * Evaluate every criterion.
   *
   * Every criterion is evaluated, without short circuiting, because a criterion reduces over the
   * communicator and skipping one on a subset of the ranks would deadlock.
   *
   * @return whether any criterion fires on any rank
   */
  bool criteriaFire();

  /**
   * Remesh the initial condition, before the transient starts, the way initial adaptivity does.
   *
   * Each cycle refreshes the indicators and the fields of the EXEC_INITIAL group so that the
   * criteria and the remeshers measure the current mesh, remeshes when a criterion fires, and
   * re-projects the initial conditions onto the mesh the surgery produced, so that the transient
   * starts from the exact initial fields on a mesh that already meets them. The cycles stop early
   * once no criterion fires. Skipped on a restarted or recovered run, whose mesh is the one it was
   * checkpointed with.
   *
   * Called at the end of initialSetup(), which is after the initial conditions were projected and
   * the EXEC_INITIAL group was executed for the first time.
   */
  void initialRemesh();

  /**
   * Run the surgery of every remesher, in the order the remeshers were declared in, each one on the
   * mesh the one before it left.
   */
  void remesh();

  /**
   * Carry the state across the surgery one remesher performed and rebuild everything that depends
   * on the mesh.
   *
   * A record names entities of the mesh it was built on, and the rebuild renumbers and frees those
   * entities, so the surgery of one remesher is carried across and rebuilt before the next remesher
   * is asked for its own rather than the records of several being merged.
   *
   * @param record the record of the surgery one remesher just performed
   */
  void applyRecord(const Remesher::RemeshRecord & record);

  /// The problem being remeshed
  FEProblemBase & _fe_problem;

  /// The reference mesh
  MooseMesh & _mesh;

  /// The displaced problem, when the problem has true displacements
  std::shared_ptr<DisplacedProblem> _displaced_problem;

  /// The executioner driving the engine, taken from the check initialSetup() already performs
  const TransientBase * _executioner;

  /// on/off flag reporting if remeshing is being used
  bool _on;

  /// Whether a smoother moves the mesh between remesh events
  bool _mesh_movement;

  /// The displacement variables of the problem
  std::vector<VariableName> _displacements;

  /// The number of time steps between two evaluations of the criteria
  unsigned int _check_interval;

  /// The largest number of remesh cycles performed on the initial condition, zero for none
  unsigned int _initial_remesh_cycles;

  /// The objects that perform the surgery, run in the order they were declared in
  std::vector<std::shared_ptr<Remesher>> _remeshers;

  /// The tests that decide when to remesh
  std::vector<std::shared_ptr<RemeshCriterion>> _criteria;

  /// The object that updates the pseudo-displacement, only used when _mesh_movement
  std::shared_ptr<MeshSmootherBase> _smoother;

  /// Maps every solution state from the replaced elements onto the new ones
  std::unique_ptr<RemeshTransfer> _transfer;

  /// The reference node coordinates X0 at the last remesh
  PointMap & _reference_coordinates;

  /// The total pseudo-displacement d since the last remesh
  PointMap & _pseudo_displacement;

  /// The number of remesh events performed so far
  unsigned int & _remesh_count;

  /// d as it stood at the start of the current attempt at a time step. A remesh event clears it,
  /// because the surgery leaves it keyed by node ids that no longer name the nodes it was taken
  /// over
  PointMap _attempt_pseudo_displacement;

  /// Whether a remesh event fired during the current attempt at a time step
  bool _attempt_remeshed;

  /// The time step size the current attempt at a time step moved the mesh with
  Real _attempt_dt;
};
