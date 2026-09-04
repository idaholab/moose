//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Remeshing.h"

#include "DisplacedProblem.h"
#include "Exodus.h"
#include "FEProblemBase.h"
#include "MeshSmootherBase.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseMeshUtils.h"
#include "OutputWarehouse.h"
#include "RemeshCriterion.h"
#include "RemeshTransfer.h"
#include "Remesher.h"
#include "TransientBase.h"

#ifdef LIBMESH_ENABLE_AMR
#include "Adaptivity.h"
#endif

#include "libmesh/elem.h"
#include "libmesh/elem_range.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/utility.h"

#include <algorithm>
#include <utility>

using namespace libMesh;

namespace
{
/// The X0 and the d of one node, pulled as a pair so that a node never arrives with one half of
/// x = X0 + d taken from the owner and the other half missing
using NodeBookkeeping = std::pair<Point, Point>;
}

Remeshing::Remeshing(FEProblemBase & fe_problem)
  : ConsoleStreamInterface(fe_problem.getMooseApp()),
    PerfGraphInterface(fe_problem.getMooseApp().perfGraph(), "Remeshing"),
    ParallelObject(fe_problem.getMooseApp()),
    Restartable(fe_problem.getMooseApp(), "Remeshing", "Remeshing", 0),
    _fe_problem(fe_problem),
    _mesh(_fe_problem.mesh()),
    _executioner(nullptr),
    _on(false),
    _mesh_movement(false),
    _check_interval(1),
    _initial_remesh_cycles(0),
    _reference_coordinates(declareRestartableData<PointMap>("reference_coordinates")),
    _pseudo_displacement(declareRestartableData<PointMap>("pseudo_displacement")),
    _remesh_count(declareRestartableData<unsigned int>("remesh_count", 0)),
    _attempt_remeshed(false),
    _attempt_dt(0)
{
}

Remeshing::~Remeshing() {}

void
Remeshing::init(const bool mesh_movement,
                const std::vector<VariableName> & displacements,
                const unsigned int check_interval,
                const unsigned int initial_remesh_cycles)
{
  // The displaced problem does not exist yet at construction, which is why it is picked up here
  _displaced_problem = _fe_problem.getDisplacedProblem();

  _mesh_movement = mesh_movement;
  _displacements = displacements;
  _check_interval = check_interval;
  _initial_remesh_cycles = initial_remesh_cycles;

  _on = true;
}

void
Remeshing::addRemesher(std::shared_ptr<Remesher> remesher)
{
  _remeshers.push_back(std::move(remesher));
}

void
Remeshing::addCriterion(std::shared_ptr<RemeshCriterion> criterion)
{
  _criteria.push_back(std::move(criterion));
}

void
Remeshing::addSmoother(std::shared_ptr<MeshSmootherBase> smoother)
{
  if (_smoother)
    mooseError("The [Remeshing] block accepts a single mesh smoother, but both '",
               _smoother->name(),
               "' and '",
               smoother->name(),
               "' were added.");

  _smoother = std::move(smoother);
}

void
Remeshing::initialSetup()
{
  if (!_on)
    return;

#ifdef LIBMESH_ENABLE_AMR
  if (_fe_problem.adaptivity().willModifyMesh())
    mooseError("The [Adaptivity] and [Remeshing] systems cannot both be active in the same "
               "simulation. Both of them replace the mesh, and their solution transfer paths are "
               "incompatible. Remove either the [Adaptivity] block or the [Remeshing] block.");
#endif

  // remeshingStep() is driven from TransientBase::takeStep(), so any other executioner would
  // accept this block, set everything up, and then never move or remesh anything. The executioner
  // is kept because remeshingStep() has to ask it whether the attempt it is repeating converged.
  _executioner = dynamic_cast<const TransientBase *>(_fe_problem.getMooseApp().getExecutioner());
  if (!_executioner)
    mooseError("The [Remeshing] system requires a transient executioner, because the engine is "
               "driven from the start of every time step.");

  // Not covered by a regression test: the XFEM module is not linked into the framework test
  // application, so no input here can reach this refusal
  if (_fe_problem.haveXFEM())
    mooseError(
        "The [Remeshing] and XFEM systems cannot both be active in the same simulation.\n\nXFEM "
        "repeats a time step through a mechanism that reports a converged solve, so the engine "
        "cannot recognize the repeat: it identifies one by the executioner reporting that the "
        "previous solve failed, which is how it restores the pseudo-displacement the step started "
        "from. The pseudo-displacement would instead advance a second time for the same step and "
        "the mesh would be moved twice as far, with no diagnostic.\n\nSupporting the combination "
        "requires the engine to learn the step-repeat contract of XFEM, which is separate work. "
        "Remove either the [Remeshing] block or XFEM.");

  if (_remeshers.empty())
    mooseError("The [Remeshing] block requires at least one remesher.");

  if (_criteria.empty())
    mooseError("The [Remeshing] block requires at least one remesh criterion.");

  if (_mesh_movement && !_smoother)
    mooseError("The [Remeshing] block sets mesh_movement = true, which requires a mesh smoother to "
               "update the pseudo-displacement.");

  if (!_mesh_movement && _smoother)
    mooseError("The [Remeshing] block defines the mesh smoother '",
               _smoother->name(),
               "', which is only used when mesh_movement = true.");

  // Refused at setup rather than survived at run time: the renumbering Exodus output performs
  // reaches the engine through FEProblemBase::meshChanged() alone, which it does not yet rebuild
  // its node-id-keyed bookkeeping on
  if (_mesh.isDistributedMesh())
  {
    // A disabled output never writes and so never renumbers. The warehouse holds it either way, so
    // it is filtered on the same condition OutputWarehouse::outputStep() gates on.
    const auto exodus_outputs = _fe_problem.getMooseApp().getOutputWarehouse().getOutputs<Exodus>();
    const auto enabled_exodus =
        std::find_if(exodus_outputs.begin(),
                     exodus_outputs.end(),
                     [](const Exodus * const output) { return output->enabled(); });
    if (enabled_exodus != exodus_outputs.end())
      mooseError(
          "The [Remeshing] system cannot be combined with the Exodus output '",
          (*enabled_exodus)->name(),
          "' on a distributed mesh.\n\nExodus output serializes the mesh and renumbers it whenever "
          "it is not contiguously numbered, which is exactly the state a remesh event leaves a "
          "distributed mesh in. The reference coordinates and the pseudo-displacement are keyed by "
          "node id, so that renumbering leaves every key naming a different node and the mesh "
          "would be moved to silently wrong positions.\n\nUse Nemesis output, which writes one "
          "file per processor and neither serializes nor renumbers the mesh, or output the "
          "quantities of interest with CSV. Exodus output stays available on a replicated mesh, "
          "whose node ids are never renumbered.");
  }

  // Fail before the first solve if a stateful material property would be silently dropped by a
  // remesh event
  _transfer = std::make_unique<RemeshTransfer>(_fe_problem);
  _transfer->checkStatefulCoverage();

  // A restarted simulation resumes with the X0 and d it was checkpointed with
  if (_reference_coordinates.empty())
    snapshotReferenceCoordinates();

  // The objects are set up last because a smoother sizes its system from X0
  for (const auto & criterion : _criteria)
    criterion->initialSetup();
  for (const auto & remesher : _remeshers)
    remesher->initialSetup();
  if (_smoother)
    _smoother->initialSetup();

  // A restarted or recovered run resumes on the mesh it was checkpointed with, which the initial
  // cycles already shaped when the original run started
  if (_initial_remesh_cycles && !_fe_problem.getMooseApp().isRecovering() &&
      !_fe_problem.getMooseApp().isRestarting())
    initialRemesh();
}

void
Remeshing::initialRemesh()
{
  TIME_SECTION("initialRemesh", 2, "Performing Initial Remeshing");

  const bool needs_indicators =
      std::any_of(_criteria.begin(),
                  _criteria.end(),
                  [](const auto & criterion) { return criterion->consumesIndicators(); });

  // Refresh what the criteria and the remeshers measure on the current mesh: the indicator fields,
  // and then the EXEC_INITIAL group, whose auxiliary kernels compute the fields that read the
  // indicators, such as a target size field. initialSetup() executed that group once already, but
  // before any indicator was computed, so the first refresh here is not redundant.
  const auto refresh = [this, needs_indicators]()
  {
    if (needs_indicators)
      _fe_problem.computeIndicators();
    _fe_problem.execute(EXEC_INITIAL);
  };

  const unsigned int count_before = _remesh_count;
  for (const auto cycle : make_range(_initial_remesh_cycles))
  {
    libmesh_ignore(cycle);

    refresh();
    if (!criteriaFire())
      break;

    remesh();

    // The transfer already carried the initial fields over, but sampled on the old mesh, and the
    // exact initial conditions are available, so they are projected the way initial adaptivity
    // re-projects them
    _fe_problem.projectSolution();
  }

  if (_remesh_count == count_before)
    return;

  // The cycle budget can run out right after a remesh, which leaves the indicators and the
  // EXEC_INITIAL fields measured on the mesh before it
  refresh();

  // The old solution states were copied before the initial cycles ran, so they still sample the
  // initial conditions on the mesh the cycles replaced
  _fe_problem.copySolutionsBackwards();

  _console << "Remeshing: initial remeshing performed " << _remesh_count - count_before
           << " remesh cycles before the transient" << std::endl;
}

void
Remeshing::snapshotReferenceCoordinates()
{
  _reference_coordinates.clear();
  _pseudo_displacement.clear();

  for (const auto & node : _mesh.getMesh().node_ptr_range())
  {
    _reference_coordinates[node->id()] = *node;
    _pseudo_displacement[node->id()] = Point();
  }
}

void
Remeshing::checkBookkeepingKeys() const
{
  // Renumbering is only ever enabled on a distributed mesh, because MooseMesh::init() turns it off
  // for every other kind
  if (!_mesh.isDistributedMesh())
    return;

  for (const auto & node : _mesh.getMesh().node_ptr_range())
  {
    const auto it = _reference_coordinates.find(node->id());
    if (it == _reference_coordinates.end())
      continue;

    // applyPseudoDisplacement() assigns x = X0 + d component by component and nothing between then
    // and here moves the reference mesh, so a key that still names its own node reproduces that sum
    // to the bit. The threshold only keeps the test off the last bits of a coordinate; a key that
    // has come to name a different node misses by a fraction of the mesh rather than by an ulp.
    constexpr Real key_identity_tol = 1e-8;
    const Point expected = it->second + libmesh_map_find(_pseudo_displacement, node->id());
    if ((*node - expected).norm() > key_identity_tol * std::max(Real(1), node->norm()))
      mooseError(
          "The node ids of the reference mesh were renumbered without the remeshing engine being "
          "told. Node ",
          node->id(),
          " sits at ",
          static_cast<const Point &>(*node),
          " but the reference coordinate and the pseudo-displacement filed under its id place it "
          "at ",
          expected,
          ", so that id no longer names the node it named when they were snapshotted.\n\nX0 and d "
          "are keyed by node id, which requires the ids to be stable between two remesh events. "
          "The one renumbering known to reach a distributed mesh, the one Exodus output performs, "
          "is refused when the engine is set up, so something else renumbered this mesh and "
          "reported it through FEProblemBase::meshChanged() alone, which the engine does not yet "
          "rebuild its bookkeeping on.");
  }
}

void
Remeshing::syncGhostedBookkeeping()
{
  // A replicated mesh holds every node on every processor, so its snapshot is total and no change
  // of ghost layer can happen under it
  if (!_mesh.isDistributedMesh())
    return;

  std::map<processor_id_type, std::vector<dof_id_type>> queries;
  for (const auto & node : _mesh.getMesh().node_ptr_range())
    if (!_reference_coordinates.count(node->id()))
    {
      // A processor always holds the nodes it owns and the snapshot covers every node it holds, so
      // an owned node can only be missing if its id is not the id it was snapshotted under
      if (node->processor_id() == processor_id())
        mooseError("Node ",
                   node->id(),
                   " is owned by this processor and yet carries no reference coordinate. X0 is "
                   "snapshotted over every node the processor holds, which always includes the "
                   "nodes it owns, so the node ids must have been renumbered under the engine.");

      queries[node->processor_id()].push_back(node->id());
    }

  auto gather_functor = [this](const processor_id_type,
                               const std::vector<dof_id_type> & incoming,
                               std::vector<NodeBookkeeping> & outgoing)
  {
    outgoing.resize(incoming.size());
    for (const auto i : index_range(incoming))
      outgoing[i] = {libmesh_map_find(_reference_coordinates, incoming[i]),
                     libmesh_map_find(_pseudo_displacement, incoming[i])};
  };

  auto action_functor = [this](const processor_id_type,
                               const std::vector<dof_id_type> & ids,
                               const std::vector<NodeBookkeeping> & incoming)
  {
    mooseAssert(incoming.size() == ids.size(), "One answer per requested node was expected");

    for (const auto i : index_range(ids))
    {
      const auto & [x0, d] = incoming[i];
      _reference_coordinates[ids[i]] = x0;
      _pseudo_displacement[ids[i]] = d;
    }
  };

  // Collective: every processor takes part, including the ones that gained no node and therefore
  // send no query
  const NodeBookkeeping * example = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      _communicator, queries, gather_functor, action_functor, example);
}

void
Remeshing::applyPseudoDisplacement()
{
  for (const auto & node : _mesh.getMesh().node_ptr_range())
  {
    const auto & x0 = libmesh_map_find(_reference_coordinates, node->id());
    const auto & d = libmesh_map_find(_pseudo_displacement, node->id());
    for (const auto i : make_range(Moose::dim))
      (*node)(i) = x0(i) + d(i);
  }

  // Moving the nodes invalidated the cached geometry of the reference mesh
  _mesh.getMesh().clear_point_locator();
  if (_fe_problem.haveFV())
    _mesh.setupFiniteVolumeMeshData();

  // The displaced coordinates are recomputed from the reference coordinates plus the displacement
  // solution, so the displaced mesh follows without being touched directly
  if (_displaced_problem)
    _displaced_problem->updateMesh();
}

bool
Remeshing::criteriaFire()
{
  unsigned int fires = 0;
  for (const auto & criterion : _criteria)
    if (criterion->shouldRemesh())
      fires = 1;

  // Every rank has to reach the same conclusion, because the rebuild that follows is collective
  _communicator.max(fires);

  return fires;
}

void
Remeshing::remeshingStep(const Real dt)
{
  if (!_on)
    return;

  // Anything that serializes a distributed mesh and releases it again re-prunes the ghost layer,
  // and an Exodus output of one does exactly that. This is the first point the engine regains
  // control afterwards, and everything below reads X0 and d over the ghosted nodes. The keys are
  // checked before the gap in them is filled, because a renumbering shows up as a missing key too
  // and reporting it as a ghosting gap would hide it.
  checkBookkeepingKeys();
  syncGhostedBookkeeping();

  // The executioner records the outcome of a solve after that solve and repeats a failed step
  // without advancing the step number, so a report of failure here is this call repeating the
  // attempt that produced it rather than reporting on a step of its own
  if (_executioner->lastSolveConverged())
    beginStepAttempt(dt);
  else if (!reenterStepAttempt(dt))
    return;

  if (_mesh_movement)
  {
    TIME_SECTION("moveMesh", 2, "Moving Mesh");

    _smoother->updatePseudoDisplacement(dt);
    applyPseudoDisplacement();
  }

  if (_fe_problem.timeStep() % static_cast<int>(_check_interval))
    return;

  // An indicator field is computed by the [Adaptivity] system as it adapts, which it is not doing
  // here, so a criterion that reads one needs it refreshed on the current mesh first
  if (std::any_of(_criteria.begin(),
                  _criteria.end(),
                  [](const auto & criterion) { return criterion->consumesIndicators(); }))
    _fe_problem.computeIndicators();

  if (criteriaFire())
    remesh();
}

void
Remeshing::beginStepAttempt(const Real dt)
{
  _attempt_pseudo_displacement = _pseudo_displacement;
  _attempt_remeshed = false;
  _attempt_dt = dt;
}

bool
Remeshing::reenterStepAttempt(const Real dt)
{
  if (_attempt_remeshed)
  {
    // Exact rather than tolerant on purpose: the executioner hands a repeat the very same _dt when
    // it does not recompute one, and any other value is a different span of time
    if (dt != _attempt_dt)
      mooseError(
          "The remeshing engine cannot repeat time step ",
          _fe_problem.timeStep(),
          " with a time step size of ",
          dt,
          ", because a remesh event fired during the attempt that failed and that attempt moved "
          "the mesh with a time step size of ",
          _attempt_dt,
          ".\n\nThe surgery replaced the reference coordinates and reissued the node ids the "
          "pseudo-displacement is keyed by, and the nodes it created exist only on the mesh the "
          "attempt had already moved, so the configuration this step started from no longer "
          "exists and the mesh cannot be moved over a different span of time instead. A repeat "
          "that solves the same span of time keeps the moved and replaced mesh and is accepted.\n\n"
          "Use a time stepper that repeats a failed step with the same time step size, or make the "
          "solve of this step converge.");

    return false;
  }

  // d is the only thing the failed attempt advanced, and the coordinates are assigned from
  // x = X0 + d rather than accumulated onto, so putting d back puts the mesh back exactly. The
  // smoother reads the node positions and the previous d and holds no accumulator of its own, so
  // that is the whole of its re-entry state as well.
  _pseudo_displacement = _attempt_pseudo_displacement;
  applyPseudoDisplacement();

  // The repeat may have been handed a smaller size than the attempt it replaces, and it is the
  // size the mesh is about to be moved with that a later repeat has to match
  _attempt_dt = dt;

  return true;
}

void
Remeshing::remesh()
{
  TIME_SECTION("remesh", 2, "Remeshing");

  // The surgery of one remesher is carried across and rebuilt before the next remesher is asked for
  // its own, so each one sees the mesh the one before it left
  for (const auto & remesher : _remeshers)
  {
    const auto record = remesher->remesh();
    applyRecord(record);
  }
}

void
Remeshing::applyRecord(const Remesher::RemeshRecord & record)
{
  unsigned int changed = record.changed;
  _communicator.max(changed);
  if (!changed)
    return;

  // The record locates the new values inside the replaced elements, so the old solution has to be
  // read out while those elements, and the degrees of freedom numbered on them, still exist
  _transfer->gather(record);

  // The smoother drops its handles into the entities the surgery replaced while those are still
  // valid pointers
  if (_smoother)
    _smoother->reset();

  MeshBase & mesh = _mesh.getMesh();
  MeshBase * const displaced_mesh =
      _displaced_problem ? &_displaced_problem->mesh().getMesh() : nullptr;

  // A remesher only replaces the entities its rank owns, but the other ranks hold ghost copies of
  // some of them, and libMesh requires a topology change to be performed by every rank that holds
  // the entity. The ids are collected before the entities are freed and handed to every rank, which
  // is cheap because a remesh event replaces a few patches rather than the mesh. Every rank enters
  // these collectives, including one whose own record is empty.
  std::vector<dof_id_type> deleted_element_ids;
  std::vector<dof_id_type> deleted_node_ids;
  if (_mesh.isDistributedMesh())
  {
    for (const auto & elem : record.replaced_elements)
      deleted_element_ids.push_back(elem->id());
    for (const auto & node : record.replaced_nodes)
      deleted_node_ids.push_back(node->id());

    _communicator.allgather(deleted_element_ids);
    _communicator.allgather(deleted_node_ids);
  }

  for (const auto & elem : record.replaced_elements)
  {
    // MaterialPropertyStorage is keyed on Elem *, so the storage has to go before the pointer does
    _fe_problem.eraseElementStatefulProps(*elem);

    if (displaced_mesh)
    {
      // The remesher splices new entities into both meshes under one set of ids, so the displaced
      // counterparts are removed by id
      Elem * const displaced_elem = displaced_mesh->query_elem_ptr(elem->id());
      mooseAssert(displaced_elem,
                  "The displaced mesh has no counterpart of replaced element " << elem->id());
      displaced_elem->nullify_neighbors();
      displaced_mesh->get_boundary_info().remove(displaced_elem);
      displaced_mesh->delete_elem(displaced_elem);
    }

    elem->nullify_neighbors();
    mesh.get_boundary_info().remove(elem);
    mesh.delete_elem(elem);
  }

  for (const auto & node : record.replaced_nodes)
  {
    if (displaced_mesh)
    {
      Node * const displaced_node = displaced_mesh->query_node_ptr(node->id());
      mooseAssert(displaced_node,
                  "The displaced mesh has no counterpart of replaced node " << node->id());
      displaced_mesh->get_boundary_info().remove(displaced_node);
      displaced_mesh->delete_node(displaced_node);
    }

    mesh.get_boundary_info().remove(node);
    mesh.delete_node(node);
  }

  // Neither MooseMesh::meshChanged() nor FEProblemBase::meshChanged() re-prepares the mesh, so
  // without this the surviving elements around each cavity keep neighbor links to elements that
  // were just freed, and the new elements never get any. A distributed mesh needs the stale ghost
  // copies dropped and the new entities gathered first, which the parallel sequence does before
  // re-preparing. The displaced mesh is a mesh of its own, so it goes through the same steps.
  if (_mesh.isDistributedMesh())
  {
    MooseMeshUtils::rebuildAfterParallelElementSurgery(mesh, deleted_element_ids, deleted_node_ids);
    if (displaced_mesh)
      MooseMeshUtils::rebuildAfterParallelElementSurgery(
          *displaced_mesh, deleted_element_ids, deleted_node_ids);
  }
  else
  {
    MooseMeshUtils::rebuildAfterElementSurgery(mesh);
    if (displaced_mesh)
      MooseMeshUtils::rebuildAfterElementSurgery(*displaced_mesh);
  }

  _fe_problem.meshChanged(/*intermediate_change=*/false,
                          /*contract_mesh=*/true,
                          /*clean_refinement_flags=*/false);

  // The equation systems have been reinitialized, so the new elements finally have degrees of
  // freedom to write the gathered solution into
  _transfer->scatter(record);

  // X0 becomes the current configuration and d goes back to zero, rebuilt once the mesh has
  // reached its final node set and numbering
  snapshotReferenceCoordinates();

  // The surgery just reissued the node ids the start-of-attempt d was keyed by and replaced the X0
  // it was measured against, so it can never be restored onto this mesh and a repeat of this
  // attempt has to keep what the surgery left instead
  _attempt_pseudo_displacement.clear();
  _attempt_remeshed = true;

  // The stateful projection in meshChanged() only covers refined and coarsened elements, which the
  // surgery leaves empty, so the new elements get their stateful properties initialized here.
  // Stateful storage only ever holds local elements, and on a replicated mesh every rank recorded
  // every new element, so the range is restricted to the ones this rank owns.
  std::vector<Elem *> local_new_elements;
  for (Elem * const elem : record.new_elements)
    if (elem->processor_id() == processor_id())
      local_new_elements.push_back(elem);
  const auto new_elem_range = MooseMeshUtils::buildElemRange(local_new_elements);
  _fe_problem.initElementStatefulProps(new_elem_range, /*threaded=*/true);

  if (_smoother)
    _smoother->reinitOnNewMesh();

  ++_remesh_count;

  _console << "\nRemeshing: mesh replaced, remesh number " << _remesh_count << std::endl;
}
