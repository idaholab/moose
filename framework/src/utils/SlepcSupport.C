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
#include "petscsnes.h"
#include "slepceps.h"

using namespace libMesh;

namespace Moose
{
namespace SlepcSupport
{

const int subspace_factor = 2;

InputParameters
getSlepcValidParams(InputParameters & params)
{
  MooseEnum solve_type("POWER ARNOLDI KRYLOVSCHUR JACOBI_DAVIDSON "
                       "NONLINEAR_POWER NEWTON PJFNK PJFNKMO JFNK",
                       "PJFNK");
  params.set<MooseEnum>("solve_type") = solve_type;

  params.setDocString("solve_type",
                      "POWER: Power / Inverse / RQI "
                      "ARNOLDI: Arnoldi "
                      "KRYLOVSCHUR: Krylov-Schur "
                      "JACOBI_DAVIDSON: Jacobi-Davidson "
                      "NONLINEAR_POWER: Nonlinear Power "
                      "NEWTON: Newton "
                      "PJFNK: Preconditioned Jacobian-free Newton-Kyrlov"
                      "JFNK: Jacobian-free Newton-Kyrlov"
                      "PJFNKMO: Preconditioned Jacobian-free Newton-Kyrlov with Matrix Only");

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
                               "GEN_INDEFINITE POS_GEN_NON_HERMITIAN SLEPC_DEFAULT",
                               "GEN_NON_HERMITIAN");
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

  params.addParam<unsigned int>(
      "extra_power_iterations", 0, "The number of extra free power iterations");

  params.addParamNamesToGroup(
      "eigen_problem_type which_eigen_pairs n_eigen_pairs n_basis_vectors eigen_tol eigen_max_its "
      "free_power_iterations extra_power_iterations",
      "Eigen Solver");
  params.addParamNamesToGroup("l_abs_tol", "Linear solver");

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
    Moose::PetscSupport::setSinglePetscOption("-snes_max_it",
                                              stringify(params.get<unsigned int>("nl_max_its")));

    Moose::PetscSupport::setSinglePetscOption("-snes_max_funcs",
                                              stringify(params.get<unsigned int>("nl_max_funcs")));

    Moose::PetscSupport::setSinglePetscOption("-snes_atol",
                                              stringify(params.get<Real>("nl_abs_tol")));

    Moose::PetscSupport::setSinglePetscOption("-snes_rtol",
                                              stringify(params.get<Real>("nl_rel_tol")));

    Moose::PetscSupport::setSinglePetscOption("-snes_stol",
                                              stringify(params.get<Real>("nl_rel_step_tol")));

    // linear solver
    Moose::PetscSupport::setSinglePetscOption("-ksp_max_it",
                                              stringify(params.get<unsigned int>("l_max_its")));

    Moose::PetscSupport::setSinglePetscOption("-ksp_rtol", stringify(params.get<Real>("l_tol")));

    Moose::PetscSupport::setSinglePetscOption("-ksp_atol",
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
setEigenProblemSolverParams(EigenProblem & eigen_problem, const InputParameters & params)
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

  if (params.get<MooseEnum>("solve_type") == "PJFNK")
  {
    eigen_problem.solverParams()._eigen_matrix_free = true;
  }
  if (params.get<MooseEnum>("solve_type") == "JFNK")
  {
    eigen_problem.solverParams()._eigen_matrix_free = true;
    eigen_problem.solverParams()._precond_matrix_free = true;
  }
  // We need matrices so that we can implement residual evaluations
  if (params.get<MooseEnum>("solve_type") == "PJFNKMO")
  {
    eigen_problem.solverParams()._eigen_matrix_free = true;
    eigen_problem.solverParams()._precond_matrix_free = false;
    eigen_problem.solverParams()._eigen_matrix_vector_mult = true;
    // By default, we need to form full matrices, otherwise residual
    // evaluations will not be accurate
    eigen_problem.setCoupling(Moose::COUPLING_FULL);
  }

  eigen_problem.constantMatrices(params.get<bool>("constant_matrices"));

  if (eigen_problem.constantMatrices() && params.get<MooseEnum>("solve_type") != "PJFNKMO")
  {
    mooseError("constant_matrices flag is only valid for solve type: PJFNKMO");
  }
}

void
storeSolveType(FEProblemBase & fe_problem, const InputParameters & params)
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
setFreeNonlinearPowerIterations(unsigned int free_power_iterations)
{
  Moose::PetscSupport::setSinglePetscOption("-eps_power_update", "0");
  Moose::PetscSupport::setSinglePetscOption("-snes_max_it", "2");
  // During each power iteration, we want solver converged unless linear solver does not
  // work. We here use a really loose tolerance for this purpose.
  // -snes_no_convergence_test is a perfect option, but it was removed from PETSc
  Moose::PetscSupport::setSinglePetscOption("-snes_rtol", "0.99999999999");
  Moose::PetscSupport::setSinglePetscOption("-eps_max_it", stringify(free_power_iterations));
  // We always want the number of free power iterations respected so we don't want to stop early if
  // we've satisfied a convergence criterion. Consequently we make this tolerance very tight
  Moose::PetscSupport::setSinglePetscOption("-eps_tol", "1e-50");
}

void
clearFreeNonlinearPowerIterations(const InputParameters & params)
{
  Moose::PetscSupport::setSinglePetscOption("-eps_power_update", "1");
  Moose::PetscSupport::setSinglePetscOption("-eps_max_it", "1");
  Moose::PetscSupport::setSinglePetscOption("-snes_max_it",
                                            stringify(params.get<unsigned int>("nl_max_its")));
  Moose::PetscSupport::setSinglePetscOption("-snes_rtol",
                                            stringify(params.get<Real>("nl_rel_tol")));
  Moose::PetscSupport::setSinglePetscOption("-eps_tol", stringify(params.get<Real>("eigen_tol")));
}

void
setNewtonPetscOptions(SolverParams & solver_params, const InputParameters & params)
{
#if !SLEPC_VERSION_LESS_THAN(3, 8, 0) || !PETSC_VERSION_RELEASE
  // Whether or not we need to involve an initial inverse power
  bool initial_power = params.get<bool>("_newton_inverse_power");

  Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
  Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
  Moose::PetscSupport::setSinglePetscOption("-eps_power_update", "1");
  // Only one outer iteration in EPS is allowed when Newton/PJFNK/JFNK
  // is used as the eigen solver
  Moose::PetscSupport::setSinglePetscOption("-eps_max_it", "1");
  if (initial_power)
  {
    Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_max_it", "1");
    Moose::PetscSupport::setSinglePetscOption("-init_eps_power_ksp_rtol", "1e-2");
    Moose::PetscSupport::setSinglePetscOption(
        "-init_eps_max_it", stringify(params.get<unsigned int>("free_power_iterations")));
  }
  Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude", "");
  if (solver_params._eigen_matrix_free)
  {
    Moose::PetscSupport::setSinglePetscOption("-snes_mf_operator", "1");
    if (initial_power)
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_mf_operator", "1");
  }
  else
  {
    Moose::PetscSupport::setSinglePetscOption("-snes_mf_operator", "0");
    if (initial_power)
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_mf_operator", "0");
  }
#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
  Moose::PetscSupport::setSinglePetscOption("-st_type", "sinvert");
  if (initial_power)
    Moose::PetscSupport::setSinglePetscOption("-init_st_type", "sinvert");
#endif
#else
  mooseError("Newton-based eigenvalue solver requires SLEPc 3.7.3 or higher");
#endif
}

void
setNonlinearPowerOptions(SolverParams & solver_params)
{
#if !SLEPC_VERSION_LESS_THAN(3, 8, 0) || !PETSC_VERSION_RELEASE
  Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
  Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
  Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude", "");
  if (solver_params._eigen_matrix_free)
    Moose::PetscSupport::setSinglePetscOption("-snes_mf_operator", "1");
  else
    Moose::PetscSupport::setSinglePetscOption("-snes_mf_operator", "0");

#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
  Moose::PetscSupport::setSinglePetscOption("-st_type", "sinvert");
#endif
#else
  mooseError("Nonlinear Inverse Power requires SLEPc 3.7.3 or higher");
#endif
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
      setNonlinearPowerOptions(solver_params);
      break;

    case Moose::EST_NEWTON:
      setNewtonPetscOptions(solver_params, params);
      break;

    case Moose::EST_PJFNK:
      solver_params._eigen_matrix_free = true;
      solver_params._customized_pc_for_eigen = false;
      setNewtonPetscOptions(solver_params, params);
      break;

    case Moose::EST_JFNK:
      solver_params._eigen_matrix_free = true;
      solver_params._customized_pc_for_eigen = true;
      setNewtonPetscOptions(solver_params, params);
      break;

    case Moose::EST_PJFNKMO:
      solver_params._eigen_matrix_free = true;
      solver_params._customized_pc_for_eigen = false;
      solver_params._eigen_matrix_vector_mult = true;
      setNewtonPetscOptions(solver_params, params);
      break;

    default:
      mooseError("Unknown eigen solver type \n");
  }
}

void
slepcSetOptions(EigenProblem & eigen_problem, const InputParameters & params)
{
  Moose::PetscSupport::petscSetOptions(
      eigen_problem.getPetscOptions(), eigen_problem.solverParams(), &eigen_problem);
  // Call "SolverTolerances" first, so some solver specific tolerance such as "eps_max_it"
  // can be overriden
  setSlepcEigenSolverTolerances(eigen_problem, params);
  setEigenSolverOptions(eigen_problem.solverParams(), params);
  // when Bx norm postprocessor is provided, we switch off the sign normalization
  if (eigen_problem.bxNormProvided())
    Moose::PetscSupport::setSinglePetscOption("-eps_power_sign_normalization", "0", &eigen_problem);
  setEigenProblemOptions(eigen_problem.solverParams());
  setWhichEigenPairsOptions(eigen_problem.solverParams());
  Moose::PetscSupport::addPetscOptionsFromCommandline();
}

// For matrices A and B
PetscErrorCode
mooseEPSFormMatrices(EigenProblem & eigen_problem, EPS eps, Vec x, void * ctx)
{
  ST st;
  Mat A, B;
  PetscBool aisshell, bisshell;
  PetscFunctionBegin;

  if (eigen_problem.constantMatrices() && eigen_problem.wereMatricesFormed())
    PetscFunctionReturn(PETSC_SUCCESS);

  if (eigen_problem.onLinearSolver())
    // We reach here during linear iteration when solve type is PJFNKMO.
    // We will use the matrices assembled at the beginning of this Newton
    // iteration for the following residual evaluation.
    PetscFunctionReturn(PETSC_SUCCESS);

  NonlinearEigenSystem & eigen_nl = eigen_problem.getCurrentNonlinearEigenSystem();
  auto & sys = eigen_nl.sys();
  SNES snes = eigen_nl.getSNES();
  // Rest ST state so that we can retrieve matrices
  LibmeshPetscCallQ(EPSGetST(eps, &st));
  LibmeshPetscCallQ(STResetMatrixState(st));
  LibmeshPetscCallQ(EPSGetOperators(eps, &A, &B));
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)A, MATSHELL, &aisshell));
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)B, MATSHELL, &bisshell));
  if (aisshell || bisshell)
  {
    SETERRQ(PetscObjectComm((PetscObject)eps),
            PETSC_ERR_ARG_INCOMP,
            "A and B matrices can not be shell matrices when using PJFNKMO \n");
  }
  // Form A and B
  std::vector<Mat> mats = {A, B};
  std::vector<SparseMatrix<Number> *> libmesh_mats = {&sys.get_matrix_A(), &sys.get_matrix_B()};
  moosePetscSNESFormMatricesTags(
      snes, x, mats, libmesh_mats, ctx, {eigen_nl.nonEigenMatrixTag(), eigen_nl.eigenMatrixTag()});
  eigen_problem.wereMatricesFormed(true);
  PetscFunctionReturn(PETSC_SUCCESS);
}

namespace
{
void
updateCurrentLocalSolution(CondensedEigenSystem & sys, Vec x)
{
  auto & dof_map = sys.get_dof_map();

  PetscVector<Number> X_global(x, sys.comm());

  if (dof_map.n_constrained_dofs())
  {
    sys.copy_sub_to_super(X_global, *sys.solution);
    // Set the constrained dof values
    dof_map.enforce_constraints_exactly(sys);
    sys.update();
  }
  else
  {
    PetscVector<Number> & X_sys = *cast_ptr<PetscVector<Number> *>(sys.solution.get());

    // Use the system's update() to get a good local version of the
    // parallel solution.  This operation does not modify the incoming
    // "x" vector, it only localizes information from "x" into
    // sys.current_local_solution.
    X_global.swap(X_sys);
    sys.update();
    X_global.swap(X_sys);
  }
}

std::unique_ptr<NumericVector<Number>>
createWrappedResidual(CondensedEigenSystem & sys, Vec r)
{
  auto & dof_map = sys.get_dof_map();

  if (dof_map.n_constrained_dofs())
    return sys.solution->zero_clone();
  else
  {
    auto R = std::make_unique<PetscVector<Number>>(r, sys.comm());
    R->zero();
    return R;
  }
}

void
evaluateResidual(EigenProblem & eigen_problem, Vec x, Vec r, TagID tag)
{
  auto & nl = eigen_problem.getCurrentNonlinearEigenSystem();
  auto & sys = nl.sys();
  auto & dof_map = sys.get_dof_map();

  updateCurrentLocalSolution(sys, x);
  auto R = createWrappedResidual(sys, r);

  eigen_problem.computeResidualTag(*sys.current_local_solution.get(), *R, tag);

  R->close();

  if (dof_map.n_constrained_dofs())
  {
    PetscVector<Number> sub_r(r, sys.comm());
    sys.copy_super_to_sub(*R, sub_r);
  }
}
}

void
moosePetscSNESFormMatrixTag(
    SNES /*snes*/, Vec x, Mat eigen_mat, SparseMatrix<Number> & all_dofs_mat, void * ctx, TagID tag)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  auto & nl = eigen_problem->getCurrentNonlinearEigenSystem();
  auto & sys = nl.sys();
  auto & dof_map = sys.get_dof_map();

#ifndef NDEBUG
  auto & petsc_all_dofs_mat = cast_ref<PetscMatrix<Number> &>(all_dofs_mat);
  mooseAssert(
      !dof_map.n_constrained_dofs() == (eigen_mat == petsc_all_dofs_mat.mat()),
      "If we do not have constrained dofs, then eigen_mat and all_dofs_mat should be the same. "
      "Conversely, if we do have constrained dofs, they must be different");
#endif

  updateCurrentLocalSolution(sys, x);

  if (!eigen_problem->constJacobian())
    all_dofs_mat.zero();

  eigen_problem->computeJacobianTag(*sys.current_local_solution.get(), all_dofs_mat, tag);

  if (dof_map.n_constrained_dofs())
  {
    PetscMatrix<Number> wrapped_eigen_mat(eigen_mat, sys.comm());
    sys.copy_super_to_sub(all_dofs_mat, wrapped_eigen_mat);
  }
}

void
moosePetscSNESFormMatricesTags(SNES /*snes*/,
                               Vec x,
                               std::vector<Mat> & eigen_mats,
                               std::vector<SparseMatrix<Number> *> & all_dofs_mats,
                               void * ctx,
                               const std::set<TagID> & tags)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  auto & nl = eigen_problem->getCurrentNonlinearEigenSystem();
  auto & sys = nl.sys();
  auto & dof_map = sys.get_dof_map();

#ifndef NDEBUG
  for (const auto i : index_range(eigen_mats))
    mooseAssert(!dof_map.n_constrained_dofs() ==
                    (eigen_mats[i] == cast_ptr<PetscMatrix<Number> *>(all_dofs_mats[i])->mat()),
                "If we do not have constrained dofs, then mat and libmesh_mat should be the same. "
                "Conversely, if we do have constrained dofs, they must be different");
#endif

  updateCurrentLocalSolution(sys, x);

  for (auto * const all_dofs_mat : all_dofs_mats)
    if (!eigen_problem->constJacobian())
      all_dofs_mat->zero();

  eigen_problem->computeMatricesTags(*sys.current_local_solution.get(), all_dofs_mats, tags);

  if (dof_map.n_constrained_dofs())
    for (const auto i : index_range(eigen_mats))
    {
      PetscMatrix<Number> wrapped_eigen_mat(eigen_mats[i], sys.comm());
      sys.copy_super_to_sub(*all_dofs_mats[i], wrapped_eigen_mat);
    }
}

PetscErrorCode
mooseSlepcEigenFormFunctionMFFD(void * ctx, Vec x, Vec r)
{
  PetscErrorCode (*func)(SNES, Vec, Vec, void *);
  void * fctx;
  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getCurrentNonlinearEigenSystem();
  SNES snes = eigen_nl.getSNES();

  eigen_problem->onLinearSolver(true);

  LibmeshPetscCallQ(SNESGetFunction(snes, NULL, &func, &fctx));
  if (fctx != ctx)
  {
    SETERRQ(
        PetscObjectComm((PetscObject)snes), PETSC_ERR_ARG_INCOMP, "Contexts are not consistent \n");
  }
  LibmeshPetscCallQ((*func)(snes, x, r, ctx));

  eigen_problem->onLinearSolver(false);

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEigenFormJacobianA(SNES snes, Vec x, Mat jac, Mat pc, void * ctx)
{
  PetscBool jisshell, pisshell;
  PetscBool jismffd;

  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getCurrentNonlinearEigenSystem();
  auto & sys = eigen_nl.sys();

  // If both jacobian and preconditioning are shell matrices,
  // and then assemble them and return
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)jac, MATSHELL, &jisshell));
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)jac, MATMFFD, &jismffd));

  if (jismffd && eigen_problem->solverParams()._eigen_matrix_vector_mult)
  {
    LibmeshPetscCallQ(
        MatMFFDSetFunction(jac, Moose::SlepcSupport::mooseSlepcEigenFormFunctionMFFD, ctx));

    EPS eps = eigen_nl.getEPS();

    LibmeshPetscCallQ(mooseEPSFormMatrices(*eigen_problem, eps, x, ctx));

    if (pc != jac)
    {
      LibmeshPetscCallQ(MatAssemblyBegin(jac, MAT_FINAL_ASSEMBLY));
      LibmeshPetscCallQ(MatAssemblyEnd(jac, MAT_FINAL_ASSEMBLY));
    }
    PetscFunctionReturn(PETSC_SUCCESS);
  }

  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)pc, MATSHELL, &pisshell));
  if ((jisshell || jismffd) && pisshell)
  {
    // Just assemble matrices and return
    LibmeshPetscCallQ(MatAssemblyBegin(jac, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCallQ(MatAssemblyBegin(pc, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCallQ(MatAssemblyEnd(jac, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCallQ(MatAssemblyEnd(pc, MAT_FINAL_ASSEMBLY));

    PetscFunctionReturn(PETSC_SUCCESS);
  }

  // Jacobian and precond matrix are the same
  if (jac == pc)
  {
    if (!pisshell)
      moosePetscSNESFormMatrixTag(
          snes, x, pc, sys.get_matrix_A(), ctx, eigen_nl.precondMatrixTag());

    PetscFunctionReturn(PETSC_SUCCESS);
  }
  else
  {
    if (!jisshell && !jismffd && !pisshell) // We need to form both Jacobian and precond matrix
    {
      std::vector<Mat> mats = {jac, pc};
      std::vector<SparseMatrix<Number> *> libmesh_mats = {&sys.get_matrix_A(),
                                                          &sys.get_precond_matrix()};
      std::set<TagID> tags = {eigen_nl.nonEigenMatrixTag(), eigen_nl.precondMatrixTag()};
      moosePetscSNESFormMatricesTags(snes, x, mats, libmesh_mats, ctx, tags);
      PetscFunctionReturn(PETSC_SUCCESS);
    }
    if (!pisshell) // We need to form only precond matrix
    {
      moosePetscSNESFormMatrixTag(
          snes, x, pc, sys.get_precond_matrix(), ctx, eigen_nl.precondMatrixTag());
      LibmeshPetscCallQ(MatAssemblyBegin(jac, MAT_FINAL_ASSEMBLY));
      LibmeshPetscCallQ(MatAssemblyEnd(jac, MAT_FINAL_ASSEMBLY));
      PetscFunctionReturn(PETSC_SUCCESS);
    }
    if (!jisshell && !jismffd) // We need to form only Jacobian matrix
    {
      moosePetscSNESFormMatrixTag(
          snes, x, jac, sys.get_matrix_A(), ctx, eigen_nl.nonEigenMatrixTag());
      LibmeshPetscCallQ(MatAssemblyBegin(pc, MAT_FINAL_ASSEMBLY));
      LibmeshPetscCallQ(MatAssemblyEnd(pc, MAT_FINAL_ASSEMBLY));
      PetscFunctionReturn(PETSC_SUCCESS);
    }
  }
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEigenFormJacobianB(SNES snes, Vec x, Mat jac, Mat pc, void * ctx)
{
  PetscBool jshell, pshell;
  PetscBool jismffd;

  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getCurrentNonlinearEigenSystem();
  auto & sys = eigen_nl.sys();

  // If both jacobian and preconditioning are shell matrices,
  // and then assemble them and return
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)jac, MATSHELL, &jshell));
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)jac, MATMFFD, &jismffd));
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)pc, MATSHELL, &pshell));
  if ((jshell || jismffd) && pshell)
  {
    // Just assemble matrices and return
    LibmeshPetscCallQ(MatAssemblyBegin(jac, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCallQ(MatAssemblyBegin(pc, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCallQ(MatAssemblyEnd(jac, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCallQ(MatAssemblyEnd(pc, MAT_FINAL_ASSEMBLY));

    PetscFunctionReturn(PETSC_SUCCESS);
  }

  if (jac != pc && (!jshell && !jshell))
    SETERRQ(PetscObjectComm((PetscObject)snes),
            PETSC_ERR_ARG_INCOMP,
            "Jacobian and precond matrices should be the same for eigen kernels \n");

  moosePetscSNESFormMatrixTag(snes, x, pc, sys.get_matrix_B(), ctx, eigen_nl.eigenMatrixTag());

  if (eigen_problem->negativeSignEigenKernel())
  {
    LibmeshPetscCallQ(MatScale(pc, -1.));
  }

  PetscFunctionReturn(PETSC_SUCCESS);
}

void
moosePetscSNESFormFunction(SNES /*snes*/, Vec x, Vec r, void * ctx, TagID tag)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  evaluateResidual(*eigen_problem, x, r, tag);
}

PetscErrorCode
mooseSlepcEigenFormFunctionA(SNES snes, Vec x, Vec r, void * ctx)
{
  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getCurrentNonlinearEigenSystem();

  if (eigen_problem->solverParams()._eigen_matrix_vector_mult &&
      (eigen_problem->onLinearSolver() || eigen_problem->constantMatrices()))
  {
    EPS eps = eigen_nl.getEPS();
    Mat A;
    ST st;

    LibmeshPetscCallQ(mooseEPSFormMatrices(*eigen_problem, eps, x, ctx));

    // Rest ST state so that we can restrieve matrices
    LibmeshPetscCallQ(EPSGetST(eps, &st));
    LibmeshPetscCallQ(STResetMatrixState(st));
    LibmeshPetscCallQ(EPSGetOperators(eps, &A, NULL));

    LibmeshPetscCallQ(MatMult(A, x, r));

    PetscFunctionReturn(PETSC_SUCCESS);
  }

  moosePetscSNESFormFunction(snes, x, r, ctx, eigen_nl.nonEigenVectorTag());

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEigenFormFunctionB(SNES snes, Vec x, Vec r, void * ctx)
{
  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getCurrentNonlinearEigenSystem();

  if (eigen_problem->solverParams()._eigen_matrix_vector_mult &&
      (eigen_problem->onLinearSolver() || eigen_problem->constantMatrices()))
  {
    EPS eps = eigen_nl.getEPS();
    ST st;
    Mat B;

    LibmeshPetscCallQ(mooseEPSFormMatrices(*eigen_problem, eps, x, ctx));

    // Rest ST state so that we can restrieve matrices
    LibmeshPetscCallQ(EPSGetST(eps, &st));
    LibmeshPetscCallQ(STResetMatrixState(st));
    LibmeshPetscCallQ(EPSGetOperators(eps, NULL, &B));

    LibmeshPetscCallQ(MatMult(B, x, r));

    if (eigen_problem->bxNormProvided())
    {
      // User has provided a postprocessor. We need it updated
      updateCurrentLocalSolution(eigen_nl.sys(), x);
      eigen_problem->execute(EXEC_LINEAR);
    }
  }
  else
    moosePetscSNESFormFunction(snes, x, r, ctx, eigen_nl.eigenVectorTag());

  if (eigen_problem->negativeSignEigenKernel())
  {
    LibmeshPetscCallQ(VecScale(r, -1.));
  }

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEigenFormFunctionAB(SNES /*snes*/, Vec x, Vec Ax, Vec Bx, void * ctx)
{
  PetscFunctionBegin;

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getCurrentNonlinearEigenSystem();
  auto & sys = eigen_nl.sys();
  auto & dof_map = sys.get_dof_map();

  if (eigen_problem->solverParams()._eigen_matrix_vector_mult &&
      (eigen_problem->onLinearSolver() || eigen_problem->constantMatrices()))
  {
    EPS eps = eigen_nl.getEPS();
    ST st;
    Mat A, B;

    LibmeshPetscCallQ(mooseEPSFormMatrices(*eigen_problem, eps, x, ctx));

    // Rest ST state so that we can restrieve matrices
    LibmeshPetscCallQ(EPSGetST(eps, &st));
    LibmeshPetscCallQ(STResetMatrixState(st));

    LibmeshPetscCallQ(EPSGetOperators(eps, &A, &B));

    LibmeshPetscCallQ(MatMult(A, x, Ax));
    LibmeshPetscCallQ(MatMult(B, x, Bx));

    if (eigen_problem->negativeSignEigenKernel())
      LibmeshPetscCallQ(VecScale(Bx, -1.));

    if (eigen_problem->bxNormProvided())
    {
      // User has provided a postprocessor. We need it updated
      updateCurrentLocalSolution(sys, x);
      eigen_problem->execute(EXEC_LINEAR);
    }

    PetscFunctionReturn(PETSC_SUCCESS);
  }

  updateCurrentLocalSolution(sys, x);
  auto AX = createWrappedResidual(sys, Ax);
  auto BX = createWrappedResidual(sys, Bx);

  eigen_problem->computeResidualAB(*sys.current_local_solution.get(),
                                   *AX,
                                   *BX,
                                   eigen_nl.nonEigenVectorTag(),
                                   eigen_nl.eigenVectorTag());

  AX->close();
  BX->close();

  if (dof_map.n_constrained_dofs())
  {
    PetscVector<Number> sub_Ax(Ax, sys.comm());
    sys.copy_super_to_sub(*AX, sub_Ax);
    PetscVector<Number> sub_Bx(Bx, sys.comm());
    sys.copy_super_to_sub(*BX, sub_Bx);
  }

  if (eigen_problem->negativeSignEigenKernel())
    LibmeshPetscCallQ(VecScale(Bx, -1.));

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEigenFormNorm(SNES /*snes*/, Vec /*Bx*/, PetscReal * norm, void * ctx)
{
  PetscFunctionBegin;
  auto * const eigen_problem = static_cast<EigenProblem *>(ctx);
  *norm = eigen_problem->formNorm();
  PetscFunctionReturn(PETSC_SUCCESS);
}

void
attachCallbacksToMat(EigenProblem & eigen_problem, Mat mat, bool eigen)
{
  // Recall that we are solving the potentially nonlinear problem:
  // F(x) = A(x) - \lambda B(x) = 0
  //
  // To solve this, we can use Newton's method: J \Delta x = -F
  // Generally we will approximate J using matrix free methods. However, in order to solve the
  // linearized system efficiently, we typically will need preconditioning. Typically we will build
  // the preconditioner only from A, but we also have the option to include information from B

  // Attach the Jacobian computation function. If \p mat is the "eigen" matrix corresponding to B,
  // then attach our JacobianB computation routine, else the matrix corresponds to A, and we attach
  // the JacobianA computation routine
  LibmeshPetscCallA(
      eigen_problem.comm().get(),
      PetscObjectComposeFunction((PetscObject)mat,
                                 "formJacobian",
                                 eigen ? Moose::SlepcSupport::mooseSlepcEigenFormJacobianB
                                       : Moose::SlepcSupport::mooseSlepcEigenFormJacobianA));

  // Attach the residual computation function. If \p mat is the "eigen" matrix corresponding to B,
  // then attach our FunctionB computation routine, else the matrix corresponds to A, and we attach
  // the FunctionA computation routine
  LibmeshPetscCallA(
      eigen_problem.comm().get(),
      PetscObjectComposeFunction((PetscObject)mat,
                                 "formFunction",
                                 eigen ? Moose::SlepcSupport::mooseSlepcEigenFormFunctionB
                                       : Moose::SlepcSupport::mooseSlepcEigenFormFunctionA));

  // It's also beneficial to be able to evaluate both A and B residuals at once
  LibmeshPetscCallA(eigen_problem.comm().get(),
                    PetscObjectComposeFunction((PetscObject)mat,
                                               "formFunctionAB",
                                               Moose::SlepcSupport::mooseSlepcEigenFormFunctionAB));

  // Users may choose to provide a custom measure of the norm of B (Bx for a linear system)
  if (eigen_problem.bxNormProvided())
    LibmeshPetscCallA(eigen_problem.comm().get(),
                      PetscObjectComposeFunction((PetscObject)mat,
                                                 "formNorm",
                                                 Moose::SlepcSupport::mooseSlepcEigenFormNorm));

  // Finally we need to attach the "context" object, which is our EigenProblem, to the matrices so
  // that eventually when we get callbacks from SLEPc we can call methods on the EigenProblem
  PetscContainer container;
  LibmeshPetscCallA(eigen_problem.comm().get(),
                    PetscContainerCreate(eigen_problem.comm().get(), &container));
  LibmeshPetscCallA(eigen_problem.comm().get(),
                    PetscContainerSetPointer(container, &eigen_problem));
  LibmeshPetscCallA(
      eigen_problem.comm().get(),
      PetscObjectCompose((PetscObject)mat, "formJacobianCtx", (PetscObject)container));
  LibmeshPetscCallA(
      eigen_problem.comm().get(),
      PetscObjectCompose((PetscObject)mat, "formFunctionCtx", (PetscObject)container));
  if (eigen_problem.bxNormProvided())
    LibmeshPetscCallA(eigen_problem.comm().get(),
                      PetscObjectCompose((PetscObject)mat, "formNormCtx", (PetscObject)container));

  LibmeshPetscCallA(eigen_problem.comm().get(), PetscContainerDestroy(&container));
}

PetscErrorCode
mooseMatMult_Eigen(Mat mat, Vec x, Vec r)
{
  PetscFunctionBegin;
  void * ctx = nullptr;
  LibmeshPetscCallQ(MatShellGetContext(mat, &ctx));

  if (!ctx)
    mooseError("No context is set for shell matrix ");

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getCurrentNonlinearEigenSystem();

  evaluateResidual(*eigen_problem, x, r, eigen_nl.eigenVectorTag());

  if (eigen_problem->negativeSignEigenKernel())
    LibmeshPetscCallQ(VecScale(r, -1.));

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseMatMult_NonEigen(Mat mat, Vec x, Vec r)
{
  PetscFunctionBegin;
  void * ctx = nullptr;
  LibmeshPetscCallQ(MatShellGetContext(mat, &ctx));

  if (!ctx)
    mooseError("No context is set for shell matrix ");

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & eigen_nl = eigen_problem->getCurrentNonlinearEigenSystem();

  evaluateResidual(*eigen_problem, x, r, eigen_nl.nonEigenVectorTag());

  PetscFunctionReturn(PETSC_SUCCESS);
}

void
setOperationsForShellMat(EigenProblem & eigen_problem, Mat mat, bool eigen)
{
  LibmeshPetscCallA(eigen_problem.comm().get(), MatShellSetContext(mat, &eigen_problem));
  LibmeshPetscCallA(eigen_problem.comm().get(),
                    MatShellSetOperation(mat,
                                         MATOP_MULT,
                                         eigen ? (void (*)(void))mooseMatMult_Eigen
                                               : (void (*)(void))mooseMatMult_NonEigen));
}

PETSC_EXTERN PetscErrorCode
registerPCToPETSc()
{
  PetscFunctionBegin;

  LibmeshPetscCallQ(PCRegister("moosepc", PCCreate_MoosePC));

  PetscFunctionReturn(PETSC_SUCCESS);
}

PETSC_EXTERN PetscErrorCode
PCCreate_MoosePC(PC pc)
{
  PetscFunctionBegin;

  pc->ops->view = PCView_MoosePC;
  pc->ops->destroy = PCDestroy_MoosePC;
  pc->ops->setup = PCSetUp_MoosePC;
  pc->ops->apply = PCApply_MoosePC;

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
PCDestroy_MoosePC(PC /*pc*/)
{
  PetscFunctionBegin;
  /* We do not need to do anything right now, but later we may have some data we need to free here
   */
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
PCView_MoosePC(PC /*pc*/, PetscViewer viewer)
{
  PetscBool iascii;

  PetscFunctionBegin;
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)viewer, PETSCVIEWERASCII, &iascii));
  if (iascii)
    LibmeshPetscCallQ(PetscViewerASCIIPrintf(viewer, "  %s\n", "moosepc"));

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
PCApply_MoosePC(PC pc, Vec x, Vec y)
{
  void * ctx;
  Mat Amat, Pmat;
  PetscContainer container;

  PetscFunctionBegin;
  LibmeshPetscCallQ(PCGetOperators(pc, &Amat, &Pmat));
  LibmeshPetscCallQ(
      PetscObjectQuery((PetscObject)Pmat, "formFunctionCtx", (PetscObject *)&container));
  if (container)
    LibmeshPetscCallQ(PetscContainerGetPointer(container, &ctx));
  else
    mooseError(" Can not find a context \n");

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & nl_eigen = eigen_problem->getCurrentNonlinearEigenSystem();
  auto preconditioner = nl_eigen.preconditioner();

  if (!preconditioner)
    mooseError("There is no moose preconditioner in nonlinear eigen system \n");

  PetscVector<Number> x_vec(x, preconditioner->comm());
  PetscVector<Number> y_vec(y, preconditioner->comm());

  preconditioner->apply(x_vec, y_vec);

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
PCSetUp_MoosePC(PC pc)
{
  void * ctx;
  Mat Amat, Pmat;
  PetscContainer container;

  PetscFunctionBegin;
  LibmeshPetscCallQ(PCGetOperators(pc, &Amat, &Pmat));
  LibmeshPetscCallQ(
      PetscObjectQuery((PetscObject)Pmat, "formFunctionCtx", (PetscObject *)&container));
  if (container)
    LibmeshPetscCallQ(PetscContainerGetPointer(container, &ctx));
  else
    mooseError(" Can not find a context \n");

  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearEigenSystem & nl_eigen = eigen_problem->getCurrentNonlinearEigenSystem();
  Preconditioner<Number> * preconditioner = nl_eigen.preconditioner();

  if (!preconditioner)
    mooseError("There is no moose preconditioner in nonlinear eigen system \n");

  if (!preconditioner->initialized())
    preconditioner->init();

  preconditioner->setup();

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcStoppingTest(EPS eps,
                       PetscInt its,
                       PetscInt max_it,
                       PetscInt nconv,
                       PetscInt nev,
                       EPSConvergedReason * reason,
                       void * ctx)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);

  PetscFunctionBegin;
  LibmeshPetscCallQ(EPSStoppingBasic(eps, its, max_it, nconv, nev, reason, NULL));

  // If we do free power iteration, we need to mark the solver as converged.
  // It is because SLEPc does not offer a way to copy unconverged solution.
  // If the solver is not marked as "converged", we have no way to get solution
  // from slepc. Note marking as "converged" has no side-effects at all for us.
  // If free power iteration is used as a stand-alone solver, we won't trigger
  // as "doFreePowerIteration()" is false.
  if (eigen_problem->doFreePowerIteration() && its == max_it && *reason <= 0)
  {
    *reason = EPS_CONVERGED_USER;
    eps->nconv = 1;
  }
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEPSGetSNES(EPS eps, SNES * snes)
{
  PetscBool same, nonlinear;

  PetscFunctionBegin;
  LibmeshPetscCallQ(PetscObjectTypeCompare((PetscObject)eps, EPSPOWER, &same));

  if (!same)
    mooseError("It is not eps power, and there is no snes");

  LibmeshPetscCallQ(EPSPowerGetNonlinear(eps, &nonlinear));

  if (!nonlinear)
    mooseError("It is not a nonlinear eigen solver");

  LibmeshPetscCallQ(EPSPowerGetSNES(eps, snes));

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEPSSNESSetUpOptionPrefix(EPS eps)
{
  SNES snes;
  const char * prefix = nullptr;

  PetscFunctionBegin;
  LibmeshPetscCallQ(mooseSlepcEPSGetSNES(eps, &snes));
  // There is an extra "eps_power" in snes that users do not like it.
  // Let us remove that from snes.
  // Retrieve option prefix from EPS
  LibmeshPetscCallQ(PetscObjectGetOptionsPrefix((PetscObject)eps, &prefix));
  // Set option prefix to SNES
  LibmeshPetscCallQ(SNESSetOptionsPrefix(snes, prefix));

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEPSSNESSetCustomizePC(EPS eps)
{
  SNES snes;
  KSP ksp;
  PC pc;

  PetscFunctionBegin;
  // Get SNES from EPS
  LibmeshPetscCallQ(mooseSlepcEPSGetSNES(eps, &snes));
  // Get KSP from SNES
  LibmeshPetscCallQ(SNESGetKSP(snes, &ksp));
  // Get PC from KSP
  LibmeshPetscCallQ(KSPGetPC(ksp, &pc));
  // Set PC type
  LibmeshPetscCallQ(PCSetType(pc, "moosepc"));
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEPSSNESKSPSetPCSide(FEProblemBase & problem, EPS eps)
{
  SNES snes;
  KSP ksp;

  PetscFunctionBegin;
  // Get SNES from EPS
  LibmeshPetscCallQ(mooseSlepcEPSGetSNES(eps, &snes));
  // Get KSP from SNES
  LibmeshPetscCallQ(SNESGetKSP(snes, &ksp));

  Moose::PetscSupport::petscSetDefaultPCSide(problem, ksp);

  Moose::PetscSupport::petscSetDefaultKSPNormType(problem, ksp);
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
mooseSlepcEPSMonitor(EPS eps,
                     PetscInt its,
                     PetscInt /*nconv*/,
                     PetscScalar * eigr,
                     PetscScalar * eigi,
                     PetscReal * /*errest*/,
                     PetscInt /*nest*/,
                     void * mctx)
{
  ST st;
  PetscScalar eigenr, eigeni;

  PetscFunctionBegin;
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(mctx);
  auto & console = eigen_problem->console();

  auto inverse = eigen_problem->outputInverseEigenvalue();
  LibmeshPetscCallQ(EPSGetST(eps, &st));
  eigenr = eigr[0];
  eigeni = eigi[0];
  // Make the eigenvalue consistent with shift type
  LibmeshPetscCallQ(STBackTransform(st, 1, &eigenr, &eigeni));

  auto eigenvalue = inverse ? 1.0 / eigenr : eigenr;

  // The term "k-eigenvalue" is adopted from the neutronics community.
  console << " Iteration " << its << std::setprecision(10) << std::fixed
          << (inverse ? " k-eigenvalue = " : " eigenvalue = ") << eigenvalue << std::endl;

  PetscFunctionReturn(PETSC_SUCCESS);
}

} // namespace SlepcSupport
} // namespace moose

#endif // LIBMESH_HAVE_SLEPC
