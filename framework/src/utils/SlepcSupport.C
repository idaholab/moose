//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SLEPCSUPPORT_H
#define SLEPCSUPPORT_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_SLEPC

#include "SlepcSupport.h"
// MOOSE includes
#include "MultiMooseEnum.h"
#include "InputParameters.h"
#include "EigenProblem.h"
#include "Conversion.h"
#include "EigenProblem.h"
#include "NonlinearSystemBase.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/slepc_macro.h"

namespace Moose
{
namespace SlepcSupport
{

const int subspace_factor = 2;

InputParameters
getSlepcValidParams(InputParameters & params)
{
  MooseEnum solve_type("POWER ARNOLDI KRYLOVSCHUR JACOBI_DAVIDSON "
                       "NONLINEAR_POWER MF_NONLINEAR_POWER "
                       "MONOLITH_NEWTON MF_MONOLITH_NEWTON");
  params.set<MooseEnum>("solve_type") = solve_type;

  params.setDocString("solve_type",
                      "POWER: Power / Inverse / RQI "
                      "ARNOLDI: Arnoldi "
                      "KRYLOVSCHUR: Krylov-Schur "
                      "JACOBI_DAVIDSON: Jacobi-Davidson "
                      "NONLINEAR_POWER: Nonlinear Power "
                      "MF_NONLINEAR_POWER: Matrix-free Nonlinear Power "
                      "MONOLITH_NEWTON: Newton "
                      "MF_MONOLITH_NEWTON: Matrix-free Newton ");

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
    case Moose::EST_MF_NONLINEAR_POWER:
      Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_monitor");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_ksp_monitor");
      break;

    case Moose::EST_MONOLITH_NEWTON:
    case Moose::EST_MF_MONOLITH_NEWTON:
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
#if !SLEPC_VERSION_LESS_THAN(3, 7, 3) && !PETSC_VERSION_RELEASE
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude", "");
      Moose::PetscSupport::setSinglePetscOption("-st_type", "sinvert");
#else
      mooseError("Nonlinear Inverse Power requires SLEPc 3.7.3 or higher");
#endif
      break;

    case Moose::EST_MF_NONLINEAR_POWER:
#if !SLEPC_VERSION_LESS_THAN(3, 7, 3) && !PETSC_VERSION_RELEASE
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_mf_operator", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude", "");
      Moose::PetscSupport::setSinglePetscOption("-st_type", "sinvert");
#else
      mooseError("Matrix-free nonlinear Inverse Power requires SLEPc 3.7.3 or higher");
#endif
      break;

    case Moose::EST_MONOLITH_NEWTON:
#if !SLEPC_VERSION_LESS_THAN(3, 7, 3) && !PETSC_VERSION_RELEASE
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_update", "1");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_max_it", "1");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_ksp_rtol", "1e-2");
      Moose::PetscSupport::setSinglePetscOption(
          "-init_eps_max_it", stringify(params.get<unsigned int>("free_power_iterations")));
      Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude", "");
      Moose::PetscSupport::setSinglePetscOption("-st_type", "sinvert");
#else
      mooseError("Newton-based eigenvalue solver requires SLEPc 3.7.3 or higher");
#endif
      break;

    case Moose::EST_MF_MONOLITH_NEWTON:
#if !SLEPC_VERSION_LESS_THAN(3, 7, 3) && !PETSC_VERSION_RELEASE
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_update", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_mf_operator", "1");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_mf_operator", "1");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_snes_max_it", "1");
      Moose::PetscSupport::setSinglePetscOption("-init_eps_power_ksp_rtol", "1e-2");
      Moose::PetscSupport::setSinglePetscOption(
          "-init_eps_max_it", stringify(params.get<unsigned int>("free_power_iterations")));
      Moose::PetscSupport::setSinglePetscOption("-eps_target_magnitude", "");
      Moose::PetscSupport::setSinglePetscOption("-st_type", "sinvert");
#else
      mooseError("Matrix-free Newton-based eigenvalue solver requires SLEPc 3.7.3 or higher");
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
moosePetscSNESFormJacobian(
    SNES /*snes*/, Vec x, Mat jac, Mat pc, void * ctx, Moose::KernelType type)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearSystemBase & nl = eigen_problem->getNonlinearSystemBase();
  System & sys = nl.system();

  PetscVector<Number> X_global(x, sys.comm());

  // update local solution
  X_global.localize(*sys.current_local_solution.get());

  PetscMatrix<Number> PC(pc, sys.comm());
  PetscMatrix<Number> Jac(jac, sys.comm());

  // Set the dof maps
  PC.attach_dof_map(sys.get_dof_map());
  Jac.attach_dof_map(sys.get_dof_map());

  PC.zero();

  eigen_problem->computeJacobian(*sys.current_local_solution.get(), PC, type);

  PC.close();
  if (jac != pc)
    Jac.close();
}

PetscErrorCode
mooseSlepcEigenFormJacobianA(SNES snes, Vec x, Mat jac, Mat pc, void * ctx)
{
  PetscFunctionBegin;

  moosePetscSNESFormJacobian(snes, x, jac, pc, ctx, Moose::KT_NONEIGEN);
  PetscFunctionReturn(0);
}

PetscErrorCode
mooseSlepcEigenFormJacobianB(SNES snes, Vec x, Mat jac, Mat pc, void * ctx)
{
  PetscFunctionBegin;

  moosePetscSNESFormJacobian(snes, x, jac, pc, ctx, Moose::KT_EIGEN);
  PetscFunctionReturn(0);
}

void
moosePetscSNESFormFunction(SNES /*snes*/, Vec x, Vec r, void * ctx, Moose::KernelType type)
{
  EigenProblem * eigen_problem = static_cast<EigenProblem *>(ctx);
  NonlinearSystemBase & nl = eigen_problem->getNonlinearSystemBase();
  System & sys = nl.system();

  PetscVector<Number> X_global(x, sys.comm()), R(r, sys.comm());

  // update local solution
  X_global.localize(*sys.current_local_solution.get());

  R.zero();

  eigen_problem->computeResidualType(*sys.current_local_solution.get(), R, type);

  R.close();
}

PetscErrorCode
mooseSlepcEigenFormFunctionA(SNES snes, Vec x, Vec r, void * ctx)
{
  PetscFunctionBegin;

  moosePetscSNESFormFunction(snes, x, r, ctx, Moose::KT_NONEIGEN);
  PetscFunctionReturn(0);
}

PetscErrorCode
mooseSlepcEigenFormFunctionB(SNES snes, Vec x, Vec r, void * ctx)
{
  PetscFunctionBegin;

  moosePetscSNESFormFunction(snes, x, r, ctx, Moose::KT_EIGEN);
  PetscFunctionReturn(0);
}
} // namespace SlepcSupport
} // namespace moose

#endif // LIBMESH_HAVE_SLEPC

#endif // SLEPCSUPPORT_H
