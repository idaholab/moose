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

#include "Moose.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "DisplacedProblem.h"
// libMesh
#include "equation_systems.h"
#include "kelly_error_estimator.h"
#include "patch_recovery_error_estimator.h"
#include "fourth_error_estimators.h"

#ifdef LIBMESH_ENABLE_AMR

Adaptivity::Adaptivity(FEProblem & subproblem) :
    _subproblem(subproblem),
    _mesh(_subproblem.mesh()),
    _mesh_refinement_on(false),
    _mesh_refinement(NULL),
    _error_estimator(NULL),
    _error(NULL),
    _displaced_problem(_subproblem.getDisplacedProblem()),
    _displaced_mesh_refinement(NULL),
    _initial_steps(0),
    _steps(0),
    _print_mesh_changed(false),
    _t(_subproblem.time()),
    _start_time(-std::numeric_limits<Real>::max()),
    _stop_time(std::numeric_limits<Real>::max())
{
}

Adaptivity::~Adaptivity()
{
  delete _mesh_refinement;
  delete _error;
  delete _error_estimator;

  delete _displaced_mesh_refinement;
}

void
Adaptivity::init(unsigned int steps, unsigned int initial_steps)
{
  if (!_mesh_refinement)
    _mesh_refinement = new MeshRefinement(_mesh);

  EquationSystems & es = _subproblem.es();
  es.parameters.set<bool>("adaptivity") = true;

  _initial_steps = initial_steps;
  _steps = steps;
  _mesh_refinement_on = true;

  _error = new ErrorVector;

  _mesh_refinement->set_periodic_boundaries_ptr(_subproblem.getNonlinearSystem().dofMap().get_periodic_boundaries());

  // displaced problem
  if (_displaced_problem != NULL)
  {
    EquationSystems & displaced_es = _displaced_problem->es();
    displaced_es.parameters.set<bool>("adaptivity") = true;

    if (!_displaced_mesh_refinement)
      _displaced_mesh_refinement = new MeshRefinement(_displaced_problem->mesh());
    _displaced_mesh_refinement->set_periodic_boundaries_ptr(_subproblem.getNonlinearSystem().dofMap().get_periodic_boundaries());

    _displaced_problem->initAdaptivity();
  }
}

void
Adaptivity::setErrorEstimator(const MooseEnum & error_estimator_name)
{
  if (error_estimator_name == "KellyErrorEstimator")
    _error_estimator = new KellyErrorEstimator;
  else if (error_estimator_name == "LaplacianErrorEstimator")
    _error_estimator = new LaplacianErrorEstimator;
  else if (error_estimator_name == "PatchRecoveryErrorEstimator")
    _error_estimator = new PatchRecoveryErrorEstimator;
  else
    mooseError("Unknown error_estimator selection: " + error_estimator_name);
}

void
Adaptivity::setErrorNorm(SystemNorm & sys_norm)
{
  mooseAssert(_error_estimator != NULL, "error_estimator not initialized. Did you call init_adaptivity()?");
  _error_estimator->error_norm = sys_norm;
}

void
Adaptivity::adaptMesh()
{
  if (_mesh_refinement_on && (_start_time <= _t && _t < _stop_time))
  {
    // Compute the error for each active element
    _error_estimator->estimate_error(_subproblem.getNonlinearSystem().sys(), *_error);

    // Flag elements to be refined and coarsened
    _mesh_refinement->flag_elements_by_error_fraction (*_error);
    // Perform refinement and coarsening
    _mesh_refinement->refine_and_coarsen_elements();

    if (_displaced_problem)
    {
      // Reuse the error vector and refine the displaced mesh
      _displaced_mesh_refinement->flag_elements_by_error_fraction (*_error);
      _displaced_mesh_refinement->refine_and_coarsen_elements();
    }

    if (_print_mesh_changed)
    {
      std::cout << "\nMesh Changed:\n";
      _mesh.printInfo();
    }
  }
}

void
Adaptivity::uniformRefine(unsigned int level)
{
  // NOTE: we are using a separate object here, since adaptivity may not be on, but we need to be able to do refinements
  MeshRefinement mesh_refinement(_mesh);
  MeshRefinement displaced_mesh_refinement(_displaced_problem ? _displaced_problem->mesh() : _mesh);

  // we have to go step by step so EquationSystems::reinit() won't freak out
  for (unsigned int i = 0; i < level; i++)
  {
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

#endif //LIBMESH_ENABLE_AMR
