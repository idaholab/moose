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

// libmesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/eigen_system.h"

namespace Moose {
#if LIBMESH_HAVE_SLEPC
void assemble_matrix(EquationSystems & es, const std::string & system_name)
{
  EigenProblem * p = es.parameters.get<EigenProblem *>("_eigen_problem");
  EigenSystem & eigen_system = es.get_system<EigenSystem>(system_name);

  p->computeJacobian(*eigen_system.current_local_solution.get(), *eigen_system.matrix_A, Moose::KT_NONEIGEN);

  // if it is a generalized eigenvalue problem
  bool generalized_eigenvalue_problem = p->isGeneralizedEigenvalueProblem();
  if (generalized_eigenvalue_problem)
  {
    if (eigen_system.matrix_B)
      p->computeJacobian(*eigen_system.current_local_solution.get(), *eigen_system.matrix_B, Moose::KT_EIGEN);
    else
      mooseError("It is a generalized eigenvalue problem but matrix B is empty \n");
  }
}
#else
void assemble_matrix(EquationSystems & /*es*/, const std::string & /*system_name*/)
{
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure libMesh\n");
}
#endif /* LIBMESH_HAVE_SLEPC */
}


#if LIBMESH_HAVE_SLEPC
NonlinearEigenSystem::NonlinearEigenSystem(EigenProblem & eigen_problem, const std::string & name)
    : NonlinearSystemBase(eigen_problem, eigen_problem.es().add_system<TransientEigenSystem>(name), name),
    _transient_sys(eigen_problem.es().get_system<TransientEigenSystem>(name)),
    _n_eigen_pairs_required(eigen_problem.getNEigenPairsRequired())
{
  if (eigen_problem.isGeneralizedEigenvalueProblem())
    _transient_sys.set_eigenproblem_type(GNHEP);

  // Give the system a pointer to the matrix assembly
  // function defined below.
  sys().attach_assemble_function(Moose::assemble_matrix);
}
#else
NonlinearEigenSystem::NonlinearEigenSystem(EigenProblem & /*eigen_problem*/, const std::string & /*name*/)
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
  //return NULL;
}

NonlinearSolver<Number> *
NonlinearEigenSystem::nonlinearSolver()
{
  mooseError("did not implement yet \n");
  return NULL;
}

const std::pair<Real, Real>
NonlinearEigenSystem::getNthConvergedEigenvalue(dof_id_type n)
{
  unsigned int n_converged_eigenvalues = getNumConvergedEigenvalues();
  if (n >= n_converged_eigenvalues)
  {
    mooseError(n << " not in [0, " << n_converged_eigenvalues << ")");
  }
  return _transient_sys.get_eigenpair(n);
}


void
NonlinearEigenSystem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to warehouse
    MooseSharedPointer<KernelBase> kernel = _factory.create<KernelBase>(kernel_name, name, parameters, tid);
    _kernels.addObject(kernel, tid);

    // Store time/non-time kernels separately
    MooseSharedPointer<TimeKernel> t_kernel = MooseSharedNamespace::dynamic_pointer_cast<TimeKernel>(kernel);
    if (t_kernel)
      _time_kernels.addObject(kernel, tid);
    else
      _non_time_kernels.addObject(kernel, tid);

    // Store eigen kernels separately
    if (kernel->isEigenKernel())
      _eigen_kernels.addObject(kernel, tid);
    else
      _non_eigen_kernels.addObject(kernel, tid);
  }

  if (parameters.get<std::vector<AuxVariableName> >("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName> >("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}


#endif /* LIBMESH_HAVE_SLEPC */
