//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_SLEPC

#include "SlepcSupport.h"
// MOOSE includes
#include "MultiMooseEnum.h"
#include "InputParameters.h"
#include "Conversion.h"
#include "EigenProblem.h"
#include "FEProblemBase.h"
#include "NonlinearEigenSystem.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/slepc_macro.h"
#include "libmesh/auto_ptr.h"

namespace Moose
{
namespace SlepcSupport
{

const int subspace_factor = 2;

InputParameters
getSlepcValidParams(InputParameters & params)
{
  MooseEnum solve_type("POWER ARNOLDI KRYLOVSCHUR JACOBI_DAVIDSON "
                       "NONLINEAR_POWER NEWTON");
  params.set<MooseEnum>("solve_type") = solve_type;

  params.setDocString("solve_type",
                      "POWER: Power / Inverse / RQI "
                      "ARNOLDI: Arnoldi "
                      "KRYLOVSCHUR: Krylov-Schur "
                      "JACOBI_DAVIDSON: Jacobi-Davidson "
                      "NONLINEAR_POWER: Nonlinear Power "
                      "NEWTON: Newton ");

  // When the eigenvalue problems is reformed as a coupled nonlinear system,
  // we use part of Jacobian as the preconditioning matrix.
  // Because the difference between the Jacobian and the preconditioning matrix is not small,
  // the linear solver KSP can not reduce the residual much. After several tests,
  // we find 1e-2 is a reasonable choice.
  params.set<Real>("l_tol") = 1e-2;

  return params;
}

InputParameters
getSlepcEigenProblemValidParams()
{
  InputParameters params = emptyInputParameters();

  // We are solving a Non-Hermitian eigenvalue problem by default
  MooseEnum eigen_problem_type("HERMITIAN NON_HERMITIAN GEN_HERMITIAN GEN_NON_HERMITIAN "
                               "GEN_INDEFINITE POS_GEN_NON_HERMITIAN SLEPC_DEFAULT");
  params.addParam<MooseEnum>(
      "eigen_problem_type",
      eigen_problem_type,
      "Type of the eigenvalue problem we are solving "
      "HERMITIAN: Hermitian "
      "NON_HERMITIAN: Non-Hermitian "
      "GEN_HERMITIAN: Generalized Hermitian "
      "GEN_NON_HERMITIAN: Generalized Non-Hermitian "
      "GEN_INDEFINITE: Generalized indefinite Hermitian "
      "POS_GEN_NON_HERMITIAN: Generalized Non-Hermitian with positive (semi-)definite B "
      "SLEPC_DEFAULT: Use whatever SLEPC has by default ");

  // Which eigenvalues are we interested in
  MooseEnum which_eigen_pairs("LARGEST_MAGNITUDE SMALLEST_MAGNITUDE LARGEST_REAL SMALLEST_REAL "
                              "LARGEST_IMAGINARY SMALLEST_IMAGINARY TARGET_MAGNITUDE TARGET_REAL "
                              "TARGET_IMAGINARY ALL_EIGENVALUES SLEPC_DEFAULT");
  params.addParam<MooseEnum>("which_eigen_pairs",
                             which_eigen_pairs,
                             "Which eigenvalue pairs to obtain from the solution "
                             "LARGEST_MAGNITUDE "
                             "SMALLEST_MAGNITUDE "
                             "LARGEST_REAL "
                             "SMALLEST_REAL "
                             "LARGEST_IMAGINARY "
                             "SMALLEST_IMAGINARY "
                             "TARGET_MAGNITUDE "
                             "TARGET_REAL "
                             "TARGET_IMAGINARY "
                             "ALL_EIGENVALUES "
                             "SLEPC_DEFAULT ");

  params.addParam<unsigned int>("n_eigen_pairs", 1, "The number of eigen pairs");
  params.addParam<unsigned int>("n_basis_vectors", 3, "The dimension of eigen subspaces");

  params.addParam<Real>("eigen_tol", 1.0e-4, "Relative Tolerance for Eigen Solver");
  params.addParam<unsigned int>("eigen_max_its", 10000, "Max Iterations for Eigen Solver");

  params.addParam<Real>("l_abs_tol", 1e-50, "Absolute Tolerances for Linear Solver");

  params.addParam<unsigned int>("free_power_iterations", 4, "The number of free power iterations");

  return params;
}

void
setSlepcEigenSolverTolerances(EigenProblem & eigen_problem, const InputParameters & params)
{
  Moose::PetscSupport::setSinglePetscOption("-eps_tol", stringify(params.get<Real>("eigen_tol")));

  Moose::PetscSupport::setSinglePetscOption("-eps_max_it",
                                            stringify(params.get<unsigned int>("eigen_max_its")));

  // if it is a nonlinear eigenvalue solver, we need to set tolerances for nonlinear solver and
  // linear solver
  if (eigen_problem.isNonlinearEigenvalueSolver())
  {
    // nonlinear solver tolerances
    Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_max_it",
                                              stringify(params.get<unsigned int>("nl_max_its")));

    Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_max_funcs",
                                              stringify(params.get<unsigned int>("nl_max_funcs")));

    Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_atol",
                                              stringify(params.get<Real>("nl_abs_tol")));

    Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_rtol",
                                              stringify(params.get<Real>("nl_rel_tol")));

    Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_stol",
                                              stringify(params.get<Real>("nl_rel_step_tol")));

    // linear solver
    Moose::PetscSupport::setSinglePetscOption("-eps_power_ksp_max_it",
                                              stringify(params.get<unsigned int>("l_max_its")));

    Moose::PetscSupport::setSinglePetscOption("-eps_power_ksp_rtol",
                                              stringify(params.get<Real>("l_tol")));

    Moose::PetscSupport::setSinglePetscOption("-eps_power_ksp_atol",
                                              stringify(params.get<Real>("l_abs_tol")));
  }
  else
  { // linear eigenvalue problem
    // linear solver
    Moose::PetscSupport::setSinglePetscOption("-st_ksp_max_it",
                                              stringify(params.get<unsigned int>("l_max_its")));

    Moose::PetscSupport::setSinglePetscOption("-st_ksp_rtol", stringify(params.get<Real>("l_tol")));

    Moose::PetscSupport::setSinglePetscOption("-st_ksp_atol",
                                              stringify(params.get<Real>("l_abs_tol")));
  }
}

void
storeSlepcEigenProblemOptions(EigenProblem & eigen_problem, const InputParameters & params)
{
  const std::string & eigen_problem_type = params.get<MooseEnum>("eigen_problem_type");
  if (!eigen_problem_type.empty())
    eigen_problem.solverParams()._eigen_problem_type =
        Moose::stringToEnum<Moose::EigenProblemType>(eigen_problem_type);
  else
    mooseError("Have to specify a valid eigen problem type");

  const std::string & which_eigen_pairs = params.get<MooseEnum>("which_eigen_pairs");
  if (!which_eigen_pairs.empty())
    eigen_problem.solverParams()._which_eigen_pairs =
        Moose::stringToEnum<Moose::WhichEigenPairs>(which_eigen_pairs);

  // Set necessary parametrs used in EigenSystem::solve(),
  // i.e. the number of requested eigenpairs nev and the number
  // of basis vectors ncv used in the solution algorithm. Note that
  // ncv >= nev must hold and ncv >= 2*nev is recommended
  unsigned int n_eigen_pairs = params.get<unsigned int>("n_eigen_pairs");
  unsigned int n_basis_vectors = params.get<unsigned int>("n_basis_vectors");

  eigen_problem.setNEigenPairsRequired(n_eigen_pairs);

  eigen_problem.es().parameters.set<unsigned int>("eigenpairs") = n_eigen_pairs;

  // If the subspace dimension is too small, we increase it automatically
  if (subspace_factor * n_eigen_pairs > n_basis_vectors)
  {
    n_basis_vectors = subspace_factor * n_eigen_pairs;
    mooseWarning("Number of subspaces in Eigensolver is changed by moose because the value you set "
                 "is too small");
  }

  eigen_problem.es().parameters.set<unsigned int>("basis vectors") = n_basis_vectors;

  // Operators A and B are formed as shell matrices
  eigen_problem.solverParams()._eigen_matrix_free = params.get<bool>("matrix_free");

  // Preconditioning is formed as a shell matrix
  eigen_problem.solverParams()._precond_matrix_free = params.get<bool>("precond_matrix_free");
}

void
storeSlepcOptions(FEProblemBase & fe_problem, const InputParameters & params)
{
  if (!(dynamic_cast<EigenProblem *>(&fe_problem)))
    return;

  if (params.isParamValid("solve_type"))
  {
    fe_problem.solverParams()._eigen_solve_type =
        Moose::stringToEnum<Moose::EigenSolveType>(params.get<MooseEnum>("solve_type"));
  }
}

void
setEigenProblemOptions(SolverParams & solver_params)
{
  switch (solver_params._eigen_problem_type)
  {
    case Moose::EPT_HERMITIAN:
      Moose::PetscSupport::setSinglePetscOption("-eps_hermitian");
      break;

    case Moose::EPT_NON_HERMITIAN:
      Moose::PetscSupport::setSinglePetscOption("-eps_non_hermitian");
      break;

    case Moose::EPT_GEN_HERMITIAN:
      Moose::PetscSupport::setSinglePetscOption("-eps_gen_hermitian");
      break;

    case Moose::EPT_GEN_INDEFINITE:
      Moose::PetscSupport::setSinglePetscOption("-eps_gen_indefinite");
      break;

    case Moose::EPT_GEN_NON_HERMITIAN:
      Moose::PetscSupport::setSinglePetscOption("-eps_gen_non_hermitian");
      break;

    case Moose::EPT_POS_GEN_NON_HERMITIAN:
      Moose::PetscSupport::setSinglePetscOption("-eps_pos_gen_non_hermitian");
      break;

    case Moose::EPT_SLEPC_DEFAULT:
      break;

    default:
      mooseError("Unknown eigen solver type \n");
  }
}

void
setSlepcOutputOptions(EigenProblem & eigen_problem)
{
  Moose::PetscSupport::setSinglePetscOption("-eps_monitor_conv");
  Moose::PetscSupport::setSinglePetscOption("-eps_monitor");
  switch (eigen_problem.solverParams()._eigen_solve_type)
  {
    case Moose::EST_NONLINEAR_POWER:
      Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_monitor");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_ksp_monitor");
      break;

    case Moose::EST_NEWTON:
      Moose::PetscSupport::setSinglePetscOption("-init_eps_monitor_conv");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_monitor");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_monitor");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_ksp_monitor");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_monitor");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_ksp_monitor");
      break;
    case Moose::EST_POWER:
      break;

    case Moose::EST_ARNOLDI:
      break;

    case Moose::EST_KRYLOVSCHUR:
      break;

    case Moose::EST_JACOBI_DAVIDSON:
      break;
  }
}

void
setWhichEigenPairsOptions(SolverParams & solver_params)
{
  switch (solver_params._which_eigen_pairs)
  {
    case Moose::WEP_LARGEST_MAGNITUDE:
      Moose::PetscSupport::setSinglePetscOption("-eps_largest_magnitude");
      break;

    case Moose::WEP_SMALLEST_MAGNITUDE:
      Moose::PetscSupport::setSinglePetscOption("-eps_smallest_magnitude");
      break;

    case Moose::WEP_LARGEST_REAL:
      Moose::PetscSupport::setSinglePetscOption("-eps_largest_real");
      break;

    case Moose::WEP_SMALLEST_REAL:
      Moose::PetscSupport::setSinglePetscOption("-eps_smallest_real");
      break;

    case Moose::WEP_LARGEST_IMAGINARY:
      Moose::PetscSupport::setSinglePetscOption("-eps_largest_imaginary");
      break;

    case Moose::WEP_SMALLEST_IMAGINARY:
      Moose::PetscSupport::setSinglePetscOption("-eps_smallest_imaginary");
      break;

    case Moose::WEP_TARGET_MAGNITUDE:
      Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude");
      break;

    case Moose::WEP_TARGET_REAL:
      Moose::PetscSupport::setSinglePetscOption("-eps_target_real");
      break;

    case Moose::WEP_TARGET_IMAGINARY:
      Moose::PetscSupport::setSinglePetscOption("-eps_target_imaginary");
      break;

    case Moose::WEP_ALL_EIGENVALUES:
      Moose::PetscSupport::setSinglePetscOption("-eps_all");
      break;

    case Moose::WEP_SLEPC_DEFAULT:
      break;

    default:
      mooseError("Unknown type of WhichEigenPairs \n");
  }
}

void
setEigenSolverOptions(SolverParams & solver_params, const InputParameters & params)
{
  // Avoid unused variable warnings when you have SLEPc but not PETSc-dev.
  libmesh_ignore(params);

  switch (solver_params._eigen_solve_type)
  {
    case Moose::EST_POWER:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      break;

    case Moose::EST_ARNOLDI:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "arnoldi");
      break;

    case Moose::EST_KRYLOVSCHUR:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "krylovschur");
      break;

    case Moose::EST_JACOBI_DAVIDSON:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "jd");
      break;

    case Moose::EST_NONLINEAR_POWER:
#if !SLEPC_VERSION_LESS_THAN(3, 8, 0) || !PETSC_VERSION_RELEASE
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude", "");
      if (solver_params._eigen_matrix_free)
        Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_mf_operator", "1");

      if (solver_params._customized_pc_for_eigen)
        Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_pc_type", "moosepc");

#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
      Moose::PetscSupport::setSinglePetscOption("-st_type", "sinvert");
#endif
#else
      mooseError("Nonlinear Inverse Power requires SLEPc 3.7.3 or higher");
#endif
      break;
    case Moose::EST_NEWTON:
#if !SLEPC_VERSION_LESS_THAN(3, 8, 0) || !PETSC_VERSION_RELEASE
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_update", "1");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_max_it", "1");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_ksp_rtol", "1e-2");
      Moose::PetscSupport::setSinglePetscOption(
          "-init_eps_max_it", stringify(params.get<unsigned int>("free_power_iterations")));
      Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude", "");
      if (solver_params._eigen_matrix_free)
      {
        Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_mf_operator", "1");
        Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_mf_operator", "1");
      }

      if (solver_params._customized_pc_for_eigen)
      {
        Moose::PetscSupport::setSinglePetscOption("-eps_power_pc_type", "moosepc");
        Moose::PetscSupport::setSinglePetscOption("-init_eps_power_pc_type", "moosepc");
      }
#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
      Moose::PetscSupport::setSinglePetscOption("-st_type", "sinvert");
      Moose::PetscSupport::setSinglePetscOption("-init_st_type", "sinvert");
#endif
#else
      mooseError("Newton-based eigenvalue solver requires SLEPc 3.7.3 or higher");
#endif
      break;
    default:
      mooseError("Unknown eigen solver type \n");
  }
}

void
slepcSetOptions(EigenProblem & eigen_problem, const InputParameters & params)
{
  Moose::PetscSupport::petscSetOptions(eigen_problem);
  setEigenSolverOptions(eigen_problem.solverParams(), params);
  setEigenProblemOptions(eigen_problem.solverParams());
  setWhichEigenPairsOptions(eigen_problem.solverParams());
  setSlepcEigenSolverTolerances(eigen_problem, params);
  setSlepcOutputOptions(eigen_problem);
  Moose::PetscSupport::addPetscOptionsFromCommandline();
}

void
moosePetscSNESFormMatrixTag(SNES /*snes*/, Vec x, Mat mat, void * ctx, TagID tag)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearSystemBase & nl = eigen_problem->getNonlinearSystemBase();
  System & sys = nl.system();

  PetscVector<Number> X_global(x, sys.comm());

  PetscVector<Number> & X_sys = *cast_ptr<PetscVector<Number> *>(sys.solution.get());

  // Use the system's update() to get a good local version of the
  // parallel solution.  This operation does not modify the incoming
  // "x" vector, it only localizes information from "x" into
  // sys.current_local_solution.
  X_global.swap(X_sys);
  sys.update();
  X_global.swap(X_sys);

  PetscMatrix<Number> libmesh_mat(mat, sys.comm());

  // Set the dof maps
  libmesh_mat.attach_dof_map(sys.get_dof_map());

  libmesh_mat.zero();

  eigen_problem->computeJacobianTag(*sys.current_local_solution.get(), libmesh_mat, tag);
}

void
moosePetscSNESFormMatricesTags(
    SNES /*snes*/, Vec x, std::vector<Mat> & mats, void * ctx, const std::set<TagID> & tags)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearSystemBase & nl = eigen_problem->getNonlinearSystemBase();
  System & sys = nl.system();

  PetscVector<Number> X_global(x, sys.comm());

  PetscVector<Number> & X_sys = *cast_ptr<PetscVector<Number> *>(sys.solution.get());

  // Use the system's update() to get a good local version of the
  // parallel solution.  This operation does not modify the incoming
  // "x" vector, it only localizes information from "x" into
  // sys.current_local_solution.
  X_global.swap(X_sys);
  sys.update();
  X_global.swap(X_sys);

  std::vector<std::unique_ptr<SparseMatrix<Number>>> jacobians;

  for (auto & mat : mats)
  {
    jacobians.emplace_back(libmesh_make_unique<PetscMatrix<Number>>(mat, sys.comm()));
    jacobians.back()->attach_dof_map(sys.get_dof_map());
    jacobians.back()->zero();
  }

  eigen_problem->computeMatricesTags(*sys.current_local_solution.get(), jacobians, tags);
}

PetscErrorCode
mooseSlepcEigenFormJacobianA(SNES snes, Vec x, Mat jac, Mat pc, void * ctx)
{
  PetscBool jisshell, pisshell;
  PetscBool jismffd;
  PetscErrorCode ierr;

  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getNonlinearEigenSystem();

  // If both jacobian and preconditioning are shell matrices,
  // and then assemble them and return
  ierr = PetscObjectTypeCompare((PetscObject)jac, MATSHELL, &jisshell);
  CHKERRQ(ierr);
  ierr = PetscObjectTypeCompare((PetscObject)jac, MATMFFD, &jismffd);
  CHKERRQ(ierr);
  ierr = PetscObjectTypeCompare((PetscObject)pc, MATSHELL, &pisshell);
  CHKERRQ(ierr);
  if ((jisshell || jismffd) && pisshell)
  {
    // Just assemble matrices and return
    ierr = MatAssemblyBegin(jac, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);
    ierr = MatAssemblyBegin(pc, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);
    ierr = MatAssemblyEnd(jac, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);
    ierr = MatAssemblyEnd(pc, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);

    PetscFunctionReturn(0);
  }

  // Jacobian and precond matrix are the same
  if (jac == pc)
  {
    if (!pisshell)
      moosePetscSNESFormMatrixTag(snes, x, pc, ctx, eigen_nl.precondMatrixTag());

    PetscFunctionReturn(0);
  }
  else
  {
    if (!jisshell && !jismffd && !pisshell) // We need to form both Jacobian and precond matrix
    {
      std::vector<Mat> mats = {jac, pc};
      moosePetscSNESFormMatricesTags(
          snes, x, mats, ctx, {eigen_nl.nonEigenMatrixTag(), eigen_nl.precondMatrixTag()});
      PetscFunctionReturn(0);
    }
    if (!pisshell) // We need to form only precond matrix
    {
      moosePetscSNESFormMatrixTag(snes, x, pc, ctx, eigen_nl.precondMatrixTag());
      ierr = MatAssemblyBegin(jac, MAT_FINAL_ASSEMBLY);
      CHKERRQ(ierr);
      ierr = MatAssemblyEnd(jac, MAT_FINAL_ASSEMBLY);
      CHKERRQ(ierr);
      PetscFunctionReturn(0);
    }
    if (!jisshell && !jismffd) // We need to form only Jacobian matrix
    {
      moosePetscSNESFormMatrixTag(snes, x, jac, ctx, eigen_nl.nonEigenMatrixTag());
      ierr = MatAssemblyBegin(pc, MAT_FINAL_ASSEMBLY);
      CHKERRQ(ierr);
      ierr = MatAssemblyEnd(pc, MAT_FINAL_ASSEMBLY);
      CHKERRQ(ierr);
      PetscFunctionReturn(0);
    }
  }
  PetscFunctionReturn(0);
}

PetscErrorCode
mooseSlepcEigenFormJacobianB(SNES snes, Vec x, Mat jac, Mat pc, void * ctx)
{
  PetscBool jshell, pshell;
  PetscBool jismffd;
  PetscErrorCode ierr;

  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getNonlinearEigenSystem();

  // If both jacobian and preconditioning are shell matrices,
  // and then assemble them and return
  ierr = PetscObjectTypeCompare((PetscObject)jac, MATSHELL, &jshell);
  CHKERRQ(ierr);
  ierr = PetscObjectTypeCompare((PetscObject)jac, MATMFFD, &jismffd);
  CHKERRQ(ierr);
  ierr = PetscObjectTypeCompare((PetscObject)pc, MATSHELL, &pshell);
  CHKERRQ(ierr);
  if ((jshell || jismffd) && pshell)
  {
    // Just assemble matrices and return
    ierr = MatAssemblyBegin(jac, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);
    ierr = MatAssemblyBegin(pc, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);
    ierr = MatAssemblyEnd(jac, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);
    ierr = MatAssemblyEnd(pc, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);

    PetscFunctionReturn(0);
  }

  if (jac != pc && (!jshell && !jshell))
    SETERRQ(PetscObjectComm((PetscObject)snes),
            PETSC_ERR_ARG_INCOMP,
            "Jacobian and precond matrices should be the same for eigen kernels \n");

  moosePetscSNESFormMatrixTag(snes, x, pc, ctx, eigen_nl.eigenMatrixTag());

  if (eigen_problem->negativeSignEigenKernel())
    MatScale(pc, -1.);

  PetscFunctionReturn(0);
}

void
moosePetscSNESFormFunction(SNES /*snes*/, Vec x, Vec r, void * ctx, TagID tag)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearSystemBase & nl = eigen_problem->getNonlinearSystemBase();
  System & sys = nl.system();

  PetscVector<Number> X_global(x, sys.comm()), R(r, sys.comm());

  PetscVector<Number> & X_sys = *cast_ptr<PetscVector<Number> *>(sys.solution.get());

  // Use the system's update() to get a good local version of the
  // parallel solution.  This operation does not modify the incoming
  // "x" vector, it only localizes information from "x" into
  // sys.current_local_solution.
  X_global.swap(X_sys);
  sys.update();
  X_global.swap(X_sys);

  R.zero();

  eigen_problem->computeResidualTag(*sys.current_local_solution.get(), R, tag);

  R.close();
}

PetscErrorCode
mooseSlepcEigenFormFunctionA(SNES snes, Vec x, Vec r, void * ctx)
{
  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getNonlinearEigenSystem();

  moosePetscSNESFormFunction(snes, x, r, ctx, eigen_nl.nonEigenVectorTag());

  PetscFunctionReturn(0);
}

PetscErrorCode
mooseSlepcEigenFormFunctionB(SNES snes, Vec x, Vec r, void * ctx)
{
  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getNonlinearEigenSystem();

  moosePetscSNESFormFunction(snes, x, r, ctx, eigen_nl.eigenVectorTag());

  if (eigen_problem->negativeSignEigenKernel())
    VecScale(r, -1.);

  PetscFunctionReturn(0);
}

PetscErrorCode
mooseSlepcEigenFormFunctionAB(SNES /*snes*/, Vec x, Vec Ax, Vec Bx, void * ctx)
{
  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & nl = eigen_problem->getNonlinearEigenSystem();
  System & sys = nl.system();

  PetscVector<Number> X_global(x, sys.comm()), AX(Ax, sys.comm()), BX(Bx, sys.comm());

  // update local solution
  X_global.localize(*sys.current_local_solution.get());

  PetscVector<Number> & X_sys = *cast_ptr<PetscVector<Number> *>(sys.solution.get());

  // Use the system's update() to get a good local version of the
  // parallel solution.  This operation does not modify the incoming
  // "x" vector, it only localizes information from "x" into
  // sys.current_local_solution.
  X_global.swap(X_sys);
  sys.update();
  X_global.swap(X_sys);

  AX.zero();
  BX.zero();

  eigen_problem->computeResidualAB(
      *sys.current_local_solution.get(), AX, BX, nl.nonEigenVectorTag(), nl.eigenVectorTag());

  AX.close();
  BX.close();

  if (eigen_problem->negativeSignEigenKernel())
    VecScale(Bx, -1.);

  PetscFunctionReturn(0);
}

void
attachCallbacksToMat(EigenProblem & eigen_problem, Mat mat, bool eigen)
{
  PetscObjectComposeFunction((PetscObject)mat,
                             "formJacobian",
                             eigen ? Moose::SlepcSupport::mooseSlepcEigenFormJacobianB
                                   : Moose::SlepcSupport::mooseSlepcEigenFormJacobianA);
  PetscObjectComposeFunction((PetscObject)mat,
                             "formFunction",
                             eigen ? Moose::SlepcSupport::mooseSlepcEigenFormFunctionB
                                   : Moose::SlepcSupport::mooseSlepcEigenFormFunctionA);

  PetscObjectComposeFunction(
      (PetscObject)mat, "formFunctionAB", Moose::SlepcSupport::mooseSlepcEigenFormFunctionAB);

  PetscContainer container;
  PetscContainerCreate(eigen_problem.comm().get(), &container);
  PetscContainerSetPointer(container, &eigen_problem);
  PetscObjectCompose((PetscObject)mat, "formJacobianCtx", nullptr);
  PetscObjectCompose((PetscObject)mat, "formJacobianCtx", (PetscObject)container);
  PetscObjectCompose((PetscObject)mat, "formFunctionCtx", nullptr);
  PetscObjectCompose((PetscObject)mat, "formFunctionCtx", (PetscObject)container);
  PetscContainerDestroy(&container);
}

void
mooseMatMult(EigenProblem & eigen_problem, Vec x, Vec r, TagID tag)
{
  NonlinearSystemBase & nl = eigen_problem.getNonlinearSystemBase();
  System & sys = nl.system();

  PetscVector<Number> X_global(x, sys.comm()), R(r, sys.comm());

  // update local solution
  X_global.localize(*sys.current_local_solution.get());

  R.zero();

  eigen_problem.computeResidualTag(*sys.current_local_solution.get(), R, tag);

  R.close();
}

PetscErrorCode
mooseMatMult_Eigen(Mat mat, Vec x, Vec r)
{
  PetscFunctionBegin;
  void * ctx = nullptr;
  MatShellGetContext(mat, &ctx);

  if (!ctx)
    mooseError("No context is set for shell matrix ");

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getNonlinearEigenSystem();

  mooseMatMult(*eigen_problem, x, r, eigen_nl.eigenVectorTag());

  if (eigen_problem->negativeSignEigenKernel())
    VecScale(r, -1.);

  PetscFunctionReturn(0);
}

PetscErrorCode
mooseMatMult_NonEigen(Mat mat, Vec x, Vec r)
{
  PetscFunctionBegin;
  void * ctx = nullptr;
  MatShellGetContext(mat, &ctx);

  if (!ctx)
    mooseError("No context is set for shell matrix ");

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getNonlinearEigenSystem();

  mooseMatMult(*eigen_problem, x, r, eigen_nl.nonEigenVectorTag());

  PetscFunctionReturn(0);
}

void
setOperationsForShellMat(EigenProblem & eigen_problem, Mat mat, bool eigen)
{
  MatShellSetContext(mat, &eigen_problem);
  MatShellSetOperation(mat,
                       MATOP_MULT,
                       eigen ? (void (*)(void))mooseMatMult_Eigen
                             : (void (*)(void))mooseMatMult_NonEigen);
}

PETSC_EXTERN PetscErrorCode
registerPCToPETSc()
{
  PetscErrorCode ierr;
  PetscFunctionBegin;

  ierr = PCRegister("moosepc", PCCreate_MoosePC);
  CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

PETSC_EXTERN PetscErrorCode
PCCreate_MoosePC(PC pc)
{
  PetscFunctionBegin;

  pc->ops->view = PCView_MoosePC;
  pc->ops->destroy = PCDestroy_MoosePC;
  pc->ops->setup = PCSetUp_MoosePC;
  pc->ops->apply = PCApply_MoosePC;

  PetscFunctionReturn(0);
}

PetscErrorCode PCDestroy_MoosePC(PC /*pc*/)
{
  PetscFunctionBegin;
  /* We do not need to do anything right now, but later we may have some data we need to free here
   */
  PetscFunctionReturn(0);
}

PetscErrorCode
PCView_MoosePC(PC /*pc*/, PetscViewer viewer)
{
  PetscErrorCode ierr;
  PetscBool iascii;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)viewer, PETSCVIEWERASCII, &iascii);
  CHKERRQ(ierr);
  if (iascii)
  {
    ierr = PetscViewerASCIIPrintf(viewer, "  %s\n", "moosepc");
    CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

PetscErrorCode
PCApply_MoosePC(PC pc, Vec x, Vec y)
{
  void * ctx;
  Mat Amat, Pmat;
  PetscContainer container;
  PetscErrorCode ierr;

  ierr = PCGetOperators(pc, &Amat, &Pmat);
  CHKERRQ(ierr);
  ierr = PetscObjectQuery((PetscObject)Pmat, "formFunctionCtx", (PetscObject *)&container);
  CHKERRQ(ierr);
  if (container)
  {
    ierr = PetscContainerGetPointer(container, &ctx);
    CHKERRQ(ierr);
  }
  else
  {
    mooseError(" Can not find a context \n");
  }

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & nl_eigen = eigen_problem->getNonlinearEigenSystem();
  auto preconditioner = nl_eigen.preconditioner();

  if (!preconditioner)
    mooseError("There is no moose preconditioner in nonlinear eigen system \n");

  PetscVector<Number> x_vec(x, preconditioner->comm());
  PetscVector<Number> y_vec(y, preconditioner->comm());

  preconditioner->apply(x_vec, y_vec);

  return 0;
}

PetscErrorCode
PCSetUp_MoosePC(PC pc)
{
  void * ctx;
  PetscErrorCode ierr;
  Mat Amat, Pmat;
  PetscContainer container;

  ierr = PCGetOperators(pc, &Amat, &Pmat);
  CHKERRQ(ierr);
  ierr = PetscObjectQuery((PetscObject)Pmat, "formFunctionCtx", (PetscObject *)&container);
  CHKERRQ(ierr);
  if (container)
  {
    ierr = PetscContainerGetPointer(container, &ctx);
    CHKERRQ(ierr);
  }
  else
  {
    mooseError(" Can not find a context \n");
  }
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & nl_eigen = eigen_problem->getNonlinearEigenSystem();
  Preconditioner<Number> * preconditioner = nl_eigen.preconditioner();

  if (!preconditioner)
    mooseError("There is no moose preconditioner in nonlinear eigen system \n");

  if (!preconditioner->initialized())
    preconditioner->init();

  preconditioner->setup();

  return 0;
}

} // namespace SlepcSupport
} // namespace moose

#endif // LIBMESH_HAVE_SLEPC
