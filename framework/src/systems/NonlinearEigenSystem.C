//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearEigenSystem.h"

// MOOSE includes
#include "DirichletBC.h"
#include "EigenDirichletBC.h"
#include "ArrayDirichletBC.h"
#include "EigenArrayDirichletBC.h"
#include "EigenProblem.h"
#include "IntegratedBC.h"
#include "KernelBase.h"
#include "NodalBC.h"
#include "TimeIntegrator.h"
#include "SlepcSupport.h"
#include "DGKernelBase.h"
#include "ScalarKernel.h"

#include "libmesh/eigen_system.h"
#include "libmesh/libmesh_config.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_shell_matrix.h"

#if LIBMESH_HAVE_SLEPC

namespace Moose
{

void
assemble_matrix(EquationSystems & es, const std::string & system_name)
{
  EigenProblem * p = es.parameters.get<EigenProblem *>("_eigen_problem");
  EigenSystem & eigen_system = es.get_system<EigenSystem>(system_name);
  NonlinearEigenSystem & eigen_nl = p->getNonlinearEigenSystem();

  // If this is a nonlinear eigenvalue problem,
  // we do not need to assemble anything
  if (p->isNonlinearEigenvalueSolver())
  {
    // If you want an efficient eigensolver,
    // please use PETSc 3.13 or newer.
    // We need to do an unnecessary assembly,
    // if you use PETSc that is older than 3.13.
#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
    if (eigen_system.matrix_B)
      p->computeJacobianTag(*eigen_system.current_local_solution.get(),
                            *eigen_system.matrix_B,
                            eigen_nl.eigenMatrixTag());
#endif
    return;
  }

#if !PETSC_RELEASE_LESS_THAN(3, 13, 0)
  // If we use shell matrices and do not use a shell preconditioning matrix,
  // we only need to form a preconditioning matrix
  if (eigen_system.use_shell_matrices() && !eigen_system.use_shell_precond_matrix())
  {
    p->computeJacobianTag(*eigen_system.current_local_solution.get(),
                          *eigen_system.precond_matrix,
                          eigen_nl.precondMatrixTag());
    return;
  }
#endif
  // If it is a linear generalized eigenvalue problem,
  // we assemble A and B together
  if (eigen_system.generalized())
  {
    p->computeJacobianAB(*eigen_system.current_local_solution.get(),
                         *eigen_system.matrix_A,
                         *eigen_system.matrix_B,
                         eigen_nl.nonEigenMatrixTag(),
                         eigen_nl.eigenMatrixTag());
#if LIBMESH_HAVE_SLEPC
    if (p->negativeSignEigenKernel())
      MatScale(static_cast<PetscMatrix<Number> &>(*eigen_system.matrix_B).mat(), -1.0);
#endif
    return;
  }

  // If it is a linear eigenvalue problem, we assemble matrix A
  {
    p->computeJacobianTag(*eigen_system.current_local_solution.get(),
                          *eigen_system.matrix_A,
                          eigen_nl.nonEigenMatrixTag());

    return;
  }
}
}

NonlinearEigenSystem::NonlinearEigenSystem(EigenProblem & eigen_problem, const std::string & name)
  : NonlinearSystemBase(
        eigen_problem, eigen_problem.es().add_system<TransientEigenSystem>(name), name),
    _transient_sys(eigen_problem.es().get_system<TransientEigenSystem>(name)),
    _eigen_problem(eigen_problem),
    _n_eigen_pairs_required(eigen_problem.getNEigenPairsRequired()),
    _work_rhs_vector_AX(addVector("work_rhs_vector_Ax", false, PARALLEL)),
    _work_rhs_vector_BX(addVector("work_rhs_vector_Bx", false, PARALLEL))
{
  sys().attach_assemble_function(Moose::assemble_matrix);

  _Ax_tag = eigen_problem.addVectorTag("Ax_tag");

  _Bx_tag = eigen_problem.addVectorTag("Eigen");

  _A_tag = eigen_problem.addMatrixTag("A_tag");

  _B_tag = eigen_problem.addMatrixTag("Eigen");

  /// Forcefully init the default solution states to match those available in libMesh
  /// Must be called here because it would call virtuals in the parent class
  solutionState(_default_solution_states);

  // By default, _precond_tag and _A_tag will share the same
  // objects. If we want to include eigen contributions to
  // the preconditioning matrix, and then _precond_tag will
  // point to part of "B" objects
  _precond_tag = eigen_problem.addMatrixTag("Eigen_precond");
}

void
NonlinearEigenSystem::initialSetup()
{
  NonlinearSystemBase::initialSetup();
  // DG kernels
  addEigenTagToMooseObjects(_dg_kernels);
  // Regular kernels
  addEigenTagToMooseObjects(_kernels);
  // Nodal BCs (we do not care about IBCs)
  addEigenTagToMooseObjects(_nodal_bcs);
  // Scalar kernels
  addEigenTagToMooseObjects(_scalar_kernels);
  // IntegratedBCs
  addEigenTagToMooseObjects(_integrated_bcs);
  // If the precond matrix needs to include eigen kernels,
  // we assign precond tag to all objects except nodal BCs.
  // Mathematically speaking, we can not add eigen nodal BCs
  // since they will overwrite non-eigen nodal BCs contributions.
  if (_precond_matrix_includes_eigen)
  {
    // DG kernels
    addPrecondTagToMooseObjects(_dg_kernels);
    // Regular kernels
    addPrecondTagToMooseObjects(_kernels);
    // Scalar kernels
    addPrecondTagToMooseObjects(_scalar_kernels);
    // IntegratedBCs
    addPrecondTagToMooseObjects(_integrated_bcs);
  }
}

template <typename T>
void
NonlinearEigenSystem::addPrecondTagToMooseObjects(MooseObjectTagWarehouse<T> & warehouse)
{
  for (THREAD_ID tid = 0; tid < warehouse.numThreads(); tid++)
  {
    // Get all objects out from the warehouse
    auto & objects = warehouse.getObjects(tid);

    // Assign precond tag to all objects
    for (auto & object : objects)
      object->useMatrixTag(_precond_tag);
  }
}

template <typename T>
void
NonlinearEigenSystem::addEigenTagToMooseObjects(MooseObjectTagWarehouse<T> & warehouse)
{
  for (THREAD_ID tid = 0; tid < warehouse.numThreads(); tid++)
  {
    auto & objects = warehouse.getObjects(tid);

    for (auto & object : objects)
    {
      auto & vtags = object->getVectorTags();
      // If this is not an eigen kernel
      if (vtags.find(_Bx_tag) == vtags.end())
      {
        object->useVectorTag(_Ax_tag);
        // Noneigen Kernels
        object->useMatrixTag(_precond_tag);
      }
      else // also associate eigen matrix tag if this is a eigen kernel
        object->useMatrixTag(_B_tag);

      auto & mtags = object->getMatrixTags();
      if (mtags.find(_B_tag) == mtags.end())
      {
        object->useMatrixTag(_A_tag);
        // Noneigen Kernels
        object->useMatrixTag(_precond_tag);
      }
      else
        object->useVectorTag(_Bx_tag);
    }
  }
}

void
NonlinearEigenSystem::solve()
{
  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;
  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();

  // Now that the initial solution has ben set, potentially perform a residual/Jacobian evaluation
  // to determine variable scaling factors
  if (_automatic_scaling)
  {
    if (_compute_scaling_once)
    {
      if (!_computed_scaling)
      {
        computeScaling();
        _computed_scaling = true;
      }
    }
    else
      computeScaling();
  }

// In DEBUG mode, Libmesh will check the residual automatically. This may cause
// an error because B does not need to assembly by default.
// When PETSc is older than 3.13, we always need to do an extra assembly,
// so we do not do "close" here
#if DEBUG && !PETSC_RELEASE_LESS_THAN(3, 13, 0)
  if (_eigen_problem.isGeneralizedEigenvalueProblem() && sys().matrix_B)
    sys().matrix_B->close();
#endif
  // Solve the transient problem if we have a time integrator; the
  // steady problem if not.
  if (_time_integrator)
  {
    _time_integrator->solve();
    _time_integrator->postSolve();
  }
  else
    system().solve();

  // store eigenvalues
  unsigned int n_converged_eigenvalues = getNumConvergedEigenvalues();

  _n_eigen_pairs_required = _eigen_problem.getNEigenPairsRequired();

  if (_n_eigen_pairs_required < n_converged_eigenvalues)
    n_converged_eigenvalues = _n_eigen_pairs_required;

  _eigen_values.resize(n_converged_eigenvalues);
  for (unsigned int n = 0; n < n_converged_eigenvalues; n++)
    _eigen_values[n] = getConvergedEigenvalue(n);

  // Update the active eigenvector to the solution vector
  if (n_converged_eigenvalues)
    getConvergedEigenpair(_eigen_problem.activeEigenvalueIndex());
}

void
NonlinearEigenSystem::attachSLEPcCallbacks()
{
  // Matrix A
  if (_transient_sys.matrix_A)
  {
    Mat mat = static_cast<PetscMatrix<Number> &>(*_transient_sys.matrix_A).mat();

    Moose::SlepcSupport::attachCallbacksToMat(_eigen_problem, mat, false);

    // Tell libmesh not close matrices before solve
    _transient_sys.eigen_solver->set_close_matrix_before_solve(false);
  }

  // Matrix B
  if (_transient_sys.matrix_B)
  {
    Mat mat = static_cast<PetscMatrix<Number> &>(*_transient_sys.matrix_B).mat();

    Moose::SlepcSupport::attachCallbacksToMat(_eigen_problem, mat, true);
  }

  // Shell matrix A
  if (_transient_sys.shell_matrix_A)
  {
    Mat mat = static_cast<PetscShellMatrix<Number> &>(*_transient_sys.shell_matrix_A).mat();

    // Attach callbacks for nonlinear eigenvalue solver
    Moose::SlepcSupport::attachCallbacksToMat(_eigen_problem, mat, false);

    // Set MatMult operations for shell
    Moose::SlepcSupport::setOperationsForShellMat(_eigen_problem, mat, false);

    _transient_sys.eigen_solver->set_close_matrix_before_solve(false);
  }

  // Shell matrix B
  if (_transient_sys.shell_matrix_B)
  {
    Mat mat = static_cast<PetscShellMatrix<Number> &>(*_transient_sys.shell_matrix_B).mat();

    Moose::SlepcSupport::attachCallbacksToMat(_eigen_problem, mat, true);

    // Set MatMult operations for shell
    Moose::SlepcSupport::setOperationsForShellMat(_eigen_problem, mat, true);
  }

  // Preconditioning matrix
  if (_transient_sys.precond_matrix)
  {
    Mat mat = static_cast<PetscMatrix<Number> &>(*_transient_sys.precond_matrix).mat();

    Moose::SlepcSupport::attachCallbacksToMat(_eigen_problem, mat, true);
  }

  // Shell preconditioning matrix
  if (_transient_sys.shell_precond_matrix)
  {
    Mat mat = static_cast<PetscShellMatrix<Number> &>(*_transient_sys.shell_precond_matrix).mat();

    Moose::SlepcSupport::attachCallbacksToMat(_eigen_problem, mat, true);
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
  return _work_rhs_vector_BX;
}

NumericVector<Number> &
NonlinearEigenSystem::residualVectorAX()
{
  return _work_rhs_vector_AX;
}

NumericVector<Number> &
NonlinearEigenSystem::residualVectorBX()
{
  return _work_rhs_vector_BX;
}

NonlinearSolver<Number> *
NonlinearEigenSystem::nonlinearSolver()
{
  mooseError("did not implement yet \n");
  return NULL;
}

void
NonlinearEigenSystem::checkIntegrity()
{
  if (_nodal_bcs.hasActiveObjects())
  {
    const auto & nodal_bcs = _nodal_bcs.getActiveObjects();
    for (const auto & nodal_bc : nodal_bcs)
    {
      // If this is a dirichlet boundary condition
      auto nbc = std::dynamic_pointer_cast<DirichletBC>(nodal_bc);
      // If this is a eigen Dirichlet boundary condition
      auto eigen_nbc = std::dynamic_pointer_cast<EigenDirichletBC>(nodal_bc);
      // ArrayDirichletBC
      auto anbc = std::dynamic_pointer_cast<ArrayDirichletBC>(nodal_bc);
      // EigenArrayDirichletBC
      auto aeigen_nbc = std::dynamic_pointer_cast<EigenArrayDirichletBC>(nodal_bc);
      // If it is a Dirichlet boundary condition, then value has to be zero
      if (nbc && nbc->getParam<Real>("value"))
        mooseError(
            "Can't set an inhomogeneous Dirichlet boundary condition for eigenvalue problems.");
      // If it is an array Dirichlet boundary condition, all values should be zero
      else if (anbc)
      {
        auto & values = anbc->getParam<RealEigenVector>("values");
        for (MooseIndex(values) i = 0; i < values.size(); i++)
        {
          if (values(i))
            mooseError("Can't set an inhomogeneous array Dirichlet boundary condition for "
                       "eigenvalue problems.");
        }
      }
      else if (!nbc && !eigen_nbc && !anbc && !aeigen_nbc)
        mooseError(
            "Invalid NodalBC for eigenvalue problems, please use homogeneous (array) Dirichlet.");
    }
  }
}

std::pair<Real, Real>
NonlinearEigenSystem::getConvergedEigenvalue(dof_id_type n) const
{
  unsigned int n_converged_eigenvalues = getNumConvergedEigenvalues();
  if (n >= n_converged_eigenvalues)
    mooseError(n, " not in [0, ", n_converged_eigenvalues, ")");
  return _transient_sys.get_eigenvalue(n);
}

std::pair<Real, Real>
NonlinearEigenSystem::getConvergedEigenpair(dof_id_type n) const
{
  unsigned int n_converged_eigenvalues = getNumConvergedEigenvalues();
  if (n >= n_converged_eigenvalues)
    mooseError(n, " not in [0, ", n_converged_eigenvalues, ")");
  return _transient_sys.get_eigenpair(n);
}

void
NonlinearEigenSystem::attachPreconditioner(Preconditioner<Number> * preconditioner)
{
  _preconditioner = preconditioner;

  // If we have a customized preconditioner,
  // We need to let PETSc know that
  if (_preconditioner)
  {
    Moose::SlepcSupport::registerPCToPETSc();
    // Mark this, and then we can setup correct petsc options
    _eigen_problem.solverParams()._customized_pc_for_eigen = true;
    _eigen_problem.solverParams()._type = Moose::ST_JFNK;
  }
}

void
NonlinearEigenSystem::turnOffJacobian()
{
  // Let us do nothing at the current moment
}

void
NonlinearEigenSystem::computeScalingJacobian()
{
  _eigen_problem.computeJacobianTag(*_current_solution, _scaling_matrix, precondMatrixTag());
}

void
NonlinearEigenSystem::computeScalingResidual()
{
  _eigen_problem.computeResidualTag(*_current_solution, RHS(), nonEigenVectorTag());
}

#else

NonlinearEigenSystem::NonlinearEigenSystem(EigenProblem & eigen_problem,
                                           const std::string & /*name*/)
  : libMesh::ParallelObject(eigen_problem)
{
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure libMesh\n");
}

#endif /* LIBMESH_HAVE_SLEPC */
