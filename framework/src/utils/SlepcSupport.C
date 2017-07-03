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

InputParameters
getSlepcValidParams()
{
  InputParameters params = emptyInputParameters();

  MooseEnum eigen_solve_type("POWER ARNOLDI KRYLOVSCHUR JACOBI_DAVIDSON "
                             "NONLINEAR_POWER MF_NONLINEAR_POWER "
                             "MONOLITH_NEWTON MF_MONOLITH_NEWTON");
  params.addParam<MooseEnum>("eigen_solve_type",
                             eigen_solve_type,
                             "POWER: Power / Inverse / RQI "
                             "ARNOLDI: Arnoldi "
                             "KRYLOVSCHUR: Krylov-Schur "
                             "JACOBI_DAVIDSON: Jacobi-Davidson ");
  return params;
}

InputParameters
getSlepcEigenProblemValidParams()
{
  InputParameters params = emptyInputParameters();

  // We are solving a Non-Hermitian eigenvalue problem by default
  MooseEnum eigen_problem_type("HERMITIAN NON_HERMITIAN GEN_HERMITIAN GEN_NON_HERMITIAN "
                               "GEN_INDEFINITE POS_GEN_NON_HERMITIAN",
                               "NON_HERMITIAN");
  params.addParam<MooseEnum>(
      "eigen_problem_type",
      eigen_problem_type,
      "Type of the eigenvalue problem we are solving "
      "HERMITIAN: Hermitian "
      "NON_HERMITIAN: Non-Hermitian "
      "GEN_HERMITIAN: Generalized Hermitian "
      "GEN_NON_HERMITIAN: Generalized Non-Hermitian "
      "GEN_INDEFINITE: Generalized indefinite Hermitian "
      "POS_GEN_NON_HERMITIAN: Generalized Non-Hermitian with positive (semi-)definite B");

  // Which eigenvalues are we interested in
  MooseEnum which_eigen_pairs("LARGEST_MAGNITUDE SMALLEST_MAGNITUDE LARGEST_REAL SMALLEST_REAL "
                              "LARGEST_IMAGINARY SMALLEST_IMAGINARY TARGET_MAGNITUDE TARGET_REAL "
                              "TARGET_IMAGINARY ALL_EIGENVALUES",
                              "SMALLEST_MAGNITUDE");
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
                             "ALL_EIGENVALUES ");
  return params;
}

void
storeSlepcEigenProblemOptions(EigenProblem & eigen_problem, const InputParameters & params)
{
  const std::string & eigen_problem_type = params.get<MooseEnum>("eigen_problem_type");
  eigen_problem.solverParams()._eigen_problem_type =
      Moose::stringToEnum<Moose::EigenProblemType>(eigen_problem_type);

  const std::string & which_eigen_pairs = params.get<MooseEnum>("which_eigen_pairs");
  eigen_problem.solverParams()._which_eigen_pairs =
      Moose::stringToEnum<Moose::WhichEigenPairs>(which_eigen_pairs);
}

void
storeSlepcOptions(FEProblemBase & fe_problem, const InputParameters & params)
{
  if (!(dynamic_cast<EigenProblem *>(&fe_problem)))
    return;

  if (params.isParamValid("solve_type"))
  {
    mooseError("Can not use \"solve_type\" for eigenvalue problems, please use "
               "\"eigen_solve_type\" instead \n");
  }

  if (params.isParamValid("eigen_solve_type"))
  {
    const std::string & eigen_solve_type = params.get<MooseEnum>("eigen_solve_type");
    fe_problem.solverParams()._eigen_solve_type =
        Moose::stringToEnum<Moose::EigenSolveType>(eigen_solve_type);
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

    default:
      mooseError("Unknown eigen solver type \n");
  }
}

void
setSlepcOutputOptions()
{
  Moose::PetscSupport::setSinglePetscOption("-eps_monitor_conv");
  Moose::PetscSupport::setSinglePetscOption("-eps_monitor");
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

    default:
      mooseError("Unknown type of WhichEigenPairs \n");
  }
}

void
setEigenSolverOptions(SolverParams & solver_params)
{
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
#else
      mooseError("Nonlinear Inverse Power requires SLEPc 3.7.3 or higher");
#endif
      break;

    case Moose::EST_MF_NONLINEAR_POWER:
#if !SLEPC_VERSION_LESS_THAN(3, 7, 3) && !PETSC_VERSION_RELEASE
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_snes_mf_operator", "1");
#else
      mooseError("Matrix-free nonlinear Inverse Power requires SLEPc 3.7.3 or higher");
#endif
      break;

    case Moose::EST_MONOLITH_NEWTON:
#if !SLEPC_VERSION_LESS_THAN(3, 7, 3) && !PETSC_VERSION_RELEASE
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_nonlinear", "1");
      Moose::PetscSupport::setSinglePetscOption("-eps_power_update", "1");
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
#else
      mooseError("Matrix-free Newton-based eigenvalue solver requires SLEPc 3.7.3 or higher");
#endif
      break;

    default:
      mooseError("Unknown eigen solver type \n");
  }
}

void
slepcSetOptions(FEProblemBase & problem)
{
  Moose::PetscSupport::petscSetOptions(problem);
  setEigenSolverOptions(problem.solverParams());
  setEigenProblemOptions(problem.solverParams());
  setWhichEigenPairsOptions(problem.solverParams());
  setSlepcOutputOptions();
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
