#include "Adaptivity.h"
#include "Moose.h"
#include "Mesh.h"
#include "SubProblem.h"
#include "ImplicitSystem.h"
// libMesh
#include "equation_systems.h"
#include "kelly_error_estimator.h"
#include "fourth_error_estimators.h"

namespace Moose
{

Adaptivity::Adaptivity(SubProblem & subproblem) :
    _subproblem(subproblem),
    _mesh(_subproblem.mesh()),
    _mesh_refinement_on(false),
    _mesh_refinement(NULL),
    _error_estimator(NULL),
    _error(NULL)
{
}

Adaptivity::~Adaptivity()
{
}

void
Adaptivity::init(unsigned int steps, unsigned int initial_steps)
{
  if (!_mesh_refinement)
    _mesh_refinement = new MeshRefinement(_mesh);

  EquationSystems & es = _subproblem.es();
  es.parameters.set<bool>("adaptivity") = true;
  es.parameters.set<unsigned int>("steps") = steps;
  es.parameters.set<unsigned int>("initial_adaptivity") = initial_steps;
  _mesh_refinement_on = true;

  _error = new ErrorVector;

  _mesh_refinement->set_periodic_boundaries_ptr(_subproblem.getNonlinearSystem().dofMap().get_periodic_boundaries());
}

void
Adaptivity::setErrorEstimator(const std::string &error_estimator_name)
{
  if (error_estimator_name == "KellyErrorEstimator")
    _error_estimator = new KellyErrorEstimator;
  else if(error_estimator_name == "LaplacianErrorEstimator")
    _error_estimator = new LaplacianErrorEstimator;
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
  if (_mesh_refinement_on)
  {
    // Compute the error for each active element
    _error_estimator->estimate_error(_subproblem.getNonlinearSystem().sys(), *_error);

    // Flag elements to be refined and coarsened
    _mesh_refinement->flag_elements_by_error_fraction (*_error);

    // Perform refinement and coarsening
    _mesh_refinement->refine_and_coarsen_elements();
  }
}

}
