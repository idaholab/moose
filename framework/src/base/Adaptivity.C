/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

#ifdef LIBMESH_ENABLE_AMR

Adaptivity::Adaptivity(FEProblemBase & subproblem)
  : ConsoleStreamInterface(subproblem.getMooseApp()),
    _subproblem(subproblem),
    _mesh(_subproblem.mesh()),
    _mesh_refinement_on(false),
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

  _mesh_refinement = libmesh_make_unique<MeshRefinement>(_mesh);
  _error = libmesh_make_unique<ErrorVector>();

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
      _displaced_mesh_refinement = libmesh_make_unique<MeshRefinement>(_displaced_problem->mesh());

    // The periodic boundaries pointer allows the MeshRefinement
    // object to determine elements which are "topological" neighbors,
    // i.e. neighbors across periodic boundaries, for the purposes of
    // refinement.
    _displaced_mesh_refinement->set_periodic_boundaries_ptr(
        _subproblem.getNonlinearSystemBase().dofMap().get_periodic_boundaries());

    // TODO: This is currently an empty function on the DisplacedProblem... could it be removed?
    _displaced_problem->initAdaptivity();
  }
}

void
Adaptivity::setErrorEstimator(const MooseEnum & error_estimator_name)
{
  if (error_estimator_name == "KellyErrorEstimator")
    _error_estimator = libmesh_make_unique<KellyErrorEstimator>();
  else if (error_estimator_name == "LaplacianErrorEstimator")
    _error_estimator = libmesh_make_unique<LaplacianErrorEstimator>();
  else if (error_estimator_name == "PatchRecoveryErrorEstimator")
    _error_estimator = libmesh_make_unique<PatchRecoveryErrorEstimator>();
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
  // If the marker name is supplied, use it. Otherwise, use the one in _marker_variable_name
  if (marker_name.empty())
    marker_name = _marker_variable_name;

  bool mesh_changed = false;

  if (_use_new_system)
  {
    if (!marker_name.empty()) // Only flag if a marker variable name has been set
    {
      _mesh_refinement->clean_refinement_flags();

      std::vector<Number> serialized_solution;
      _subproblem.getAuxiliarySystem().solution().close();
      _subproblem.getAuxiliarySystem().solution().localize(serialized_solution);

      FlagElementsThread fet(_subproblem, serialized_solution, _max_h_level, marker_name);
      ConstElemRange all_elems(_subproblem.mesh().getMesh().active_elements_begin(),
                               _subproblem.mesh().getMesh().active_elements_end(),
                               1);
      Threads::parallel_reduce(all_elems, fet);
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

  // Perform refinement and coarsening
  mesh_changed = _mesh_refinement->refine_and_coarsen_elements();

  if (_displaced_problem && mesh_changed)
  {
// Now do refinement/coarsening
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
  }

  return mesh_changed;
}

bool
Adaptivity::initialAdaptMesh()
{
  return adaptMesh(_initial_marker_variable_name);
}

void
Adaptivity::uniformRefine(MooseMesh * mesh)
{
  mooseAssert(mesh, "Mesh pointer must not be NULL");

  // NOTE: we are using a separate object here, since adaptivity may not be on, but we need to be
  // able to do refinements
  MeshRefinement mesh_refinement(*mesh);
  unsigned int level = mesh->uniformRefineLevel();
  mesh_refinement.uniformly_refine(level);
}

void
Adaptivity::uniformRefineWithProjection()
{
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
  auto ev_pair_it = _indicator_field_to_error_vector.lower_bound(indicator_field);
  if (ev_pair_it == _indicator_field_to_error_vector.end() || ev_pair_it->first != indicator_field)
    ev_pair_it = _indicator_field_to_error_vector.emplace_hint(
        ev_pair_it, indicator_field, libmesh_make_unique<ErrorVector>());

  return *ev_pair_it->second;
}

void
Adaptivity::updateErrorVectors()
{
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
