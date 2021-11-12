//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Adaptivity.h"

#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "FlagElementsThread.h"
#include "MooseMesh.h"
#include "NonlinearSystemBase.h"
#include "UpdateErrorVectorsThread.h"

// libMesh
#include "libmesh/equation_systems.h"
#include "libmesh/kelly_error_estimator.h"
#include "libmesh/patch_recovery_error_estimator.h"
#include "libmesh/fourth_error_estimators.h"
#include "libmesh/parallel.h"
#include "libmesh/error_vector.h"
#include "libmesh/distributed_mesh.h"

#ifdef LIBMESH_ENABLE_AMR

Adaptivity::Adaptivity(FEProblemBase & subproblem)
  : ConsoleStreamInterface(subproblem.getMooseApp()),
    PerfGraphInterface(subproblem.getMooseApp().perfGraph(), "Adaptivity"),
    ParallelObject(subproblem.getMooseApp()),
    _subproblem(subproblem),
    _mesh(_subproblem.mesh()),
    _mesh_refinement_on(false),
    _initialized(false),
    _initial_steps(0),
    _steps(0),
    _print_mesh_changed(false),
    _t(_subproblem.time()),
    _step(_subproblem.timeStep()),
    _interval(1),
    _start_time(-std::numeric_limits<Real>::max()),
    _stop_time(std::numeric_limits<Real>::max()),
    _cycles_per_step(1),
    _use_new_system(false),
    _max_h_level(0),
    _recompute_markers_during_cycles(false)
{
}

Adaptivity::~Adaptivity() {}

void
Adaptivity::init(unsigned int steps, unsigned int initial_steps)
{
  // Get the pointer to the DisplacedProblem, this cannot be done at construction because
  // DisplacedProblem
  // does not exist at that point.
  _displaced_problem = _subproblem.getDisplacedProblem();

  _mesh_refinement = std::make_unique<MeshRefinement>(_mesh);
  _error = std::make_unique<ErrorVector>();

  EquationSystems & es = _subproblem.es();
  es.parameters.set<bool>("adaptivity") = true;

  _initial_steps = initial_steps;
  _steps = steps;
  _mesh_refinement_on = true;

  _mesh_refinement->set_periodic_boundaries_ptr(
      _subproblem.getNonlinearSystemBase().dofMap().get_periodic_boundaries());

  // displaced problem
  if (_displaced_problem != nullptr)
  {
    EquationSystems & displaced_es = _displaced_problem->es();
    displaced_es.parameters.set<bool>("adaptivity") = true;

    if (!_displaced_mesh_refinement)
      _displaced_mesh_refinement = std::make_unique<MeshRefinement>(_displaced_problem->mesh());

    // The periodic boundaries pointer allows the MeshRefinement
    // object to determine elements which are "topological" neighbors,
    // i.e. neighbors across periodic boundaries, for the purposes of
    // refinement.
    _displaced_mesh_refinement->set_periodic_boundaries_ptr(
        _subproblem.getNonlinearSystemBase().dofMap().get_periodic_boundaries());

    // TODO: This is currently an empty function on the DisplacedProblem... could it be removed?
    _displaced_problem->initAdaptivity();
  }

  // indicate the Adaptivity system has been initialized
  _initialized = true;
}

void
Adaptivity::setErrorEstimator(const MooseEnum & error_estimator_name)
{
  if (error_estimator_name == "KellyErrorEstimator")
    _error_estimator = std::make_unique<KellyErrorEstimator>();
  else if (error_estimator_name == "LaplacianErrorEstimator")
    _error_estimator = std::make_unique<LaplacianErrorEstimator>();
  else if (error_estimator_name == "PatchRecoveryErrorEstimator")
    _error_estimator = std::make_unique<PatchRecoveryErrorEstimator>();
  else
    mooseError(std::string("Unknown error_estimator selection: ") +
               std::string(error_estimator_name));
}

void
Adaptivity::setErrorNorm(SystemNorm & sys_norm)
{
  mooseAssert(_error_estimator, "error_estimator not initialized. Did you call init_adaptivity()?");
  _error_estimator->error_norm = sys_norm;
}

bool
Adaptivity::adaptMesh(std::string marker_name /*=std::string()*/)
{
  TIME_SECTION("adaptMesh", 3, "Adapting Mesh");

  // If the marker name is supplied, use it. Otherwise, use the one in _marker_variable_name
  if (marker_name.empty())
    marker_name = _marker_variable_name;

  bool mesh_changed = false;

  // If mesh adaptivity is carried out in a distributed (scalable) way
  bool distributed_adaptivity = false;

  if (_use_new_system)
  {
    if (!marker_name.empty()) // Only flag if a marker variable name has been set
    {
      _mesh_refinement->clean_refinement_flags();

      std::vector<Number> serialized_solution;

      auto distributed_mesh = dynamic_cast<DistributedMesh *>(&_subproblem.mesh().getMesh());

      // Element range
      std::unique_ptr<ConstElemRange> all_elems;
      // If the mesh is distributed and we do not do "gather to zero" or "allgather".
      // Then it is safe to not serialize solution.
      // Some output idiom (Exodus) will do "gather to zero". That being said,
      // if you have exodus output on, mesh adaptivty is not scalable.
      if (distributed_mesh && !distributed_mesh->is_serial_on_zero())
      {
        // We update here to make sure local solution is up-to-date
        _subproblem.getAuxiliarySystem().update();
        distributed_adaptivity = true;

        // We can not assume that geometric and algebraic ghosting functors cover
        // the same set of elements/nodes. That being said, in general,
        // we would expect G(e) > A(e). Here G(e) is the set of elements reserved
        // by the geometric ghosting functors, and A(e) corresponds to
        // the one covered by the algebraic ghosting functors.
        // Therefore, we have to work only on local elements instead of
        // ghosted + local elements. The ghosted solution might not be enough
        // for ghosted+local elements. But it is always sufficient for local elements.
        // After we set markers for all local elements, we will do a global
        // communication to sync markers for ghosted elements from their owners.
        all_elems = std::make_unique<ConstElemRange>(
            _subproblem.mesh().getMesh().active_local_elements_begin(),
            _subproblem.mesh().getMesh().active_local_elements_end());
      }
      else // This is not scalable but it might be useful for small-size problems
      {
        _subproblem.getAuxiliarySystem().solution().close();
        _subproblem.getAuxiliarySystem().solution().localize(serialized_solution);
        distributed_adaptivity = false;

        // For a replicated mesh or a serialized distributed mesh, the solution
        // is serialized to everyone. Then we update markers for all active elements.
        // In this case, we can avoid a global communication to update mesh.
        // I do not know if it is a good idea, but it the old code behavior.
        // We might not care about much since a replicated mesh
        // or a serialized distributed mesh is not scalable anyway.
        all_elems =
            std::make_unique<ConstElemRange>(_subproblem.mesh().getMesh().active_elements_begin(),
                                             _subproblem.mesh().getMesh().active_elements_end());
      }

      FlagElementsThread fet(
          _subproblem, serialized_solution, _max_h_level, marker_name, !distributed_adaptivity);
      Threads::parallel_reduce(*all_elems, fet);
      _subproblem.getAuxiliarySystem().solution().close();
    }
  }
  else
  {
    // Compute the error for each active element
    _error_estimator->estimate_error(_subproblem.getNonlinearSystemBase().system(), *_error);

    // Flag elements to be refined and coarsened
    _mesh_refinement->flag_elements_by_error_fraction(*_error);

    if (_displaced_problem)
      // Reuse the error vector and refine the displaced mesh
      _displaced_mesh_refinement->flag_elements_by_error_fraction(*_error);
  }

  // If the DisplacedProblem is active, undisplace the DisplacedMesh
  // in preparation for refinement.  We can't safely refine the
  // DisplacedMesh directly, since the Hilbert keys computed on the
  // inconsistenly-displaced Mesh are different on different
  // processors, leading to inconsistent Hilbert keys.  We must do
  // this before the undisplaced Mesh is refined, so that the
  // element and node numbering is still consistent.
  if (_displaced_problem)
    _displaced_problem->undisplaceMesh();

  // If markers are added to only local elements,
  // we sync them here.
  if (distributed_adaptivity)
    _mesh_refinement->make_flags_parallel_consistent();

  // Perform refinement and coarsening
  mesh_changed = _mesh_refinement->refine_and_coarsen_elements();

  if (_displaced_problem && mesh_changed)
  {
    // If markers are added to only local elements,
    // we sync them here.
    if (distributed_adaptivity)
      _displaced_mesh_refinement->make_flags_parallel_consistent();
#ifndef NDEBUG
    bool displaced_mesh_changed =
#endif
        _displaced_mesh_refinement->refine_and_coarsen_elements();

    // Since the undisplaced mesh changed, the displaced mesh better have changed!
    mooseAssert(displaced_mesh_changed, "Undisplaced mesh changed, but displaced mesh did not!");
  }

  if (mesh_changed && _print_mesh_changed)
  {
    _console << "\nMesh Changed:\n";
    _mesh.printInfo();
    _console << std::flush;
  }

  return mesh_changed;
}

bool
Adaptivity::initialAdaptMesh()
{
  return adaptMesh(_initial_marker_variable_name);
}

void
Adaptivity::uniformRefine(MooseMesh * mesh, unsigned int level /*=libMesh::invalid_uint*/)
{
  mooseAssert(mesh, "Mesh pointer must not be NULL");

  // NOTE: we are using a separate object here, since adaptivity may not be on, but we need to be
  // able to do refinements
  MeshRefinement mesh_refinement(*mesh);
  if (level == libMesh::invalid_uint)
    level = mesh->uniformRefineLevel();

  // Skip deletion and repartition will make uniform refinements will run more
  // efficiently, but at the same time, there might be extra ghosting elements.
  // The number of layers of additional ghosting elements depends on the number
  // of uniform refinement levels. This should happen only when you have a "fine enough"
  // coarse mesh and want to refine the mesh by a few levels. Otherwise, it might
  // introduce an unbalanced workload and too large ghosting domain.
  if (mesh->skipDeletionRepartitionAfterRefine())
  {
    mesh->getMesh().skip_partitioning(true);
    mesh->getMesh().allow_remote_element_removal(false);
    mesh->needsRemoteElemDeletion(false);
  }

  mesh_refinement.uniformly_refine(level);
}

void
Adaptivity::uniformRefineWithProjection()
{
  TIME_SECTION("uniformRefineWithProjection", 2, "Uniformly Refining and Reprojecting");

  // NOTE: we are using a separate object here, since adaptivity may not be on, but we need to be
  // able to do refinements
  MeshRefinement mesh_refinement(_mesh);
  unsigned int level = _mesh.uniformRefineLevel();
  MeshRefinement displaced_mesh_refinement(_displaced_problem ? _displaced_problem->mesh() : _mesh);

  // we have to go step by step so EquationSystems::reinit() won't freak out
  for (unsigned int i = 0; i < level; i++)
  {
    // See comment above about why refining the displaced mesh is potentially unsafe.
    if (_displaced_problem)
      _displaced_problem->undisplaceMesh();

    mesh_refinement.uniformly_refine(1);

    if (_displaced_problem)
      displaced_mesh_refinement.uniformly_refine(1);
    _subproblem.meshChanged();
  }
}

void
Adaptivity::setAdaptivityOn(bool state)
{
  // check if Adaptivity has been initialized before turning on
  if (state == true && !_initialized)
    mooseError("Mesh adaptivity system not available");

  _mesh_refinement_on = state;
}

void
Adaptivity::setTimeActive(Real start_time, Real stop_time)
{
  _start_time = start_time;
  _stop_time = stop_time;
}

void
Adaptivity::setUseNewSystem()
{
  _use_new_system = true;
}

void
Adaptivity::setMarkerVariableName(std::string marker_field)
{
  _marker_variable_name = marker_field;
}

void
Adaptivity::setInitialMarkerVariableName(std::string marker_field)
{
  _initial_marker_variable_name = marker_field;
}

ErrorVector &
Adaptivity::getErrorVector(const std::string & indicator_field)
{
  // Insert or retrieve error vector
  auto insert_pair = moose_try_emplace(
      _indicator_field_to_error_vector, indicator_field, std::make_unique<ErrorVector>());
  return *insert_pair.first->second;
}

void
Adaptivity::updateErrorVectors()
{
  TIME_SECTION("updateErrorVectors", 5, "Updating Error Vectors");

  // Resize all of the ErrorVectors in case the mesh has changed
  for (const auto & it : _indicator_field_to_error_vector)
  {
    ErrorVector & vec = *(it.second);
    vec.assign(_mesh.getMesh().max_elem_id(), 0);
  }

  // Fill the vectors with the local contributions
  UpdateErrorVectorsThread uevt(_subproblem, _indicator_field_to_error_vector);
  Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), uevt);

  // Now sum across all processors
  for (const auto & it : _indicator_field_to_error_vector)
    _subproblem.comm().sum((std::vector<float> &)*(it.second));
}

bool
Adaptivity::isAdaptivityDue()
{
  return _mesh_refinement_on && (_start_time <= _t && _t < _stop_time) && _step % _interval == 0;
}

#endif // LIBMESH_ENABLE_AMR
