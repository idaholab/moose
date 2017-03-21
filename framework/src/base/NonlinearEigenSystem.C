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
#include "EigenProblem.h"
#include "TimeIntegrator.h"
#include "DirichletBC.h"
#include "IntegratedBC.h"
#include "NodalBC.h"

// libmesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/eigen_system.h"

#if LIBMESH_HAVE_SLEPC

namespace Moose
{

void
assemble_matrix(EquationSystems & es, const std::string & system_name)
{
  EigenProblem * p = es.parameters.get<EigenProblem *>("_eigen_problem");
  EigenSystem & eigen_system = es.get_system<EigenSystem>(system_name);

  p->computeJacobian(
      *eigen_system.current_local_solution.get(), *eigen_system.matrix_A, Moose::KT_NONEIGEN);

  if (eigen_system.generalized())
  {
    if (eigen_system.matrix_B)
      p->computeJacobian(
          *eigen_system.current_local_solution.get(), *eigen_system.matrix_B, Moose::KT_EIGEN);
    else
      mooseError("It is a generalized eigenvalue problem but matrix B is empty\n");
  }
}
}

NonlinearEigenSystem::NonlinearEigenSystem(EigenProblem & eigen_problem, const std::string & name)
  : NonlinearSystemBase(
        eigen_problem, eigen_problem.es().add_system<TransientEigenSystem>(name), name),
    _transient_sys(eigen_problem.es().get_system<TransientEigenSystem>(name)),
    _n_eigen_pairs_required(eigen_problem.getNEigenPairsRequired())
{
  sys().attach_assemble_function(Moose::assemble_matrix);
}

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

  // store eigenvalues
  unsigned int n_converged_eigenvalues = getNumConvergedEigenvalues();

  if (_n_eigen_pairs_required < n_converged_eigenvalues)
    n_converged_eigenvalues = _n_eigen_pairs_required;

  _eigen_values.resize(n_converged_eigenvalues);
  for (unsigned int n = 0; n < n_converged_eigenvalues; n++)
    _eigen_values[n] = getNthConvergedEigenvalue(n);
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
  return _transient_sys.get_n_converged();
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
  // return NULL;
}

NonlinearSolver<Number> *
NonlinearEigenSystem::nonlinearSolver()
{
  mooseError("did not implement yet \n");
  return NULL;
}

void
NonlinearEigenSystem::addEigenKernels(std::shared_ptr<KernelBase> kernel, THREAD_ID tid)
{
  if (kernel->isEigenKernel())
    _eigen_kernels.addObject(kernel, tid);
  else
    _non_eigen_kernels.addObject(kernel, tid);
}

void
NonlinearEigenSystem::checkIntegrity()
{
  if (_integrated_bcs.hasActiveObjects())
    mooseError("Can't set an inhomogeneous integrated boundary condition for eigenvalue problems.");

  if (_nodal_bcs.hasActiveObjects())
  {
    const auto & nodal_bcs = _nodal_bcs.getActiveObjects();
    for (const auto & nodal_bc : nodal_bcs)
    {
      std::shared_ptr<DirichletBC> nbc = std::dynamic_pointer_cast<DirichletBC>(nodal_bc);
      if (nbc && nbc->getParam<Real>("value"))
        mooseError(
            "Can't set an inhomogeneous Dirichlet boundary condition for eigenvalue problems.");
      else if (!nbc)
        mooseError("Invalid NodalBC for eigenvalue problems, please use homogeneous Dirichlet.");
    }
  }
}

const std::pair<Real, Real>
NonlinearEigenSystem::getNthConvergedEigenvalue(dof_id_type n)
{
  unsigned int n_converged_eigenvalues = getNumConvergedEigenvalues();
  if (n >= n_converged_eigenvalues)
    mooseError(n, " not in [0, ", n_converged_eigenvalues, ")");
  return _transient_sys.get_eigenpair(n);
}

#else

NonlinearEigenSystem::NonlinearEigenSystem(EigenProblem & eigen_problem,
                                           const std::string & /*name*/)
  : libMesh::ParallelObject(eigen_problem)
{
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure libMesh\n");
}

#endif /* LIBMESH_HAVE_SLEPC */
