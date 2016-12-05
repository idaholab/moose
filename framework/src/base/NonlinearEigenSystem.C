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


#include "libmesh/libmesh_config.h"


// moose includes
#include "NonlinearEigenSystem.h"
#include "FEProblem.h"
#include "TimeIntegrator.h"

// libmesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/eigen_system.h"

namespace Moose {
#if LIBMESH_HAVE_SLEPC
void assemble_matrix(EquationSystems & es, const std::string & system_name)
{
  FEProblemBase * p = es.parameters.get<FEProblemBase *>("_fe_problem_base");
  EigenSystem & eigen_system = es.get_system<EigenSystem>(system_name);

  p->computeJacobian(*eigen_system.solution.get(), *eigen_system.matrix_A);
}
#else
void assemble_matrix(EquationSystems & /*es*/, const std::string & /*system_name*/)
{
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure libMesh\n");
}
#endif /* LIBMESH_HAVE_SLEPC */
}


#if LIBMESH_HAVE_SLEPC
NonlinearEigenSystem::NonlinearEigenSystem(FEProblemBase & fe_problem, const std::string & name)
    : NonlinearSystemBase(fe_problem, fe_problem.es().add_system<TransientEigenSystem>(name), name),
    _transient_sys(fe_problem.es().get_system<TransientEigenSystem>(name))
{
  // Give the system a pointer to the matrix assembly
  // function defined below.
  sys().attach_assemble_function(Moose::assemble_matrix);
}
#else
NonlinearEigenSystem::NonlinearEigenSystem(FEProblemBase & /*fe_problem*/, const std::string & /*name*/)
{
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure libMesh\n");
}
#endif /* LIBMESH_HAVE_SLEPC */


#if LIBMESH_HAVE_SLEPC
void
NonlinearEigenSystem::solve()
{
  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;
  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();
  _time_integrator->solve();
  _time_integrator->postSolve();

  // update store eigenvalues
  unsigned int n_converged_eigenvalues = getNumConvergedEigenvalues();
  _eigen_values.clear();
  _eigen_values.resize(n_converged_eigenvalues);
  for(unsigned int n = 0; n < n_converged_eigenvalues; n++)
  {
    _eigen_values.push_back(getNthConvergedEigenvalue(n));
  }
}

void
NonlinearEigenSystem::stopSolve()
{
  mooseError("did not implement yet \n");
}

void
NonlinearEigenSystem::setupFiniteDifferencedPreconditioner()
{
  mooseError("did not implement yet \n");
}

bool
NonlinearEigenSystem::converged()
{
  if (_transient_sys.get_n_converged())
    return true;
  else
    return false;
}

unsigned int
NonlinearEigenSystem::getCurrentNonlinearIterationNumber()
{
  mooseError("did not implement yet \n");
  return 0;
}

NumericVector<Number> &
NonlinearEigenSystem::RHS()
{
  mooseError("did not implement yet \n");
  //return NULL;
}

NonlinearSolver<Number> *
NonlinearEigenSystem::nonlinearSolver()
{
  mooseError("did not implement yet \n");
  return NULL;
}
#endif /* LIBMESH_HAVE_SLEPC */
