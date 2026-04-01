//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "EigenProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearEigenSystem.h"
#include "PetscSupport.h"
#include "SlepcSupport.h"
#include "UserObject.h"

#include "libmesh/petsc_solver_exception.h"

// Needed for LIBMESH_CHECK_ERR
using libMesh::PetscSolverException;

InputParameters
EigenProblemSolve::validParams()
{
  InputParameters params = Executioner::validParams();

  params.addClassDescription(
      "Solve object for a standard/generalized linear or nonlinear eigenvalue problem");

  params += FEProblemSolve::validParams();

  params.addParam<bool>(
      "matrix_free",
      false,
      "Whether or not to use a matrix free fashion to form operators. "
      "If true, shell matrices will be used and meanwhile a preconditioning matrix"
      "may be formed as well.");

  params.addParam<bool>(
      "precond_matrix_free",
      false,
      "Whether or not to use a matrix free fashion for forming the preconditioning matrix. "
      "If true, a shell matrix will be used for preconditioner.");

  params.addParam<bool>("constant_matrices",
                        false,
                        "Whether or not to use constant matrices so that we can use them to form "
                        "residuals on both linear and "
                        "nonlinear iterations");

  params.addParam<bool>(
      "precond_matrix_includes_eigen",
      false,
      "Whether or not to include eigen kernels in the preconditioning matrix. "
      "If true, the preconditioning matrix could be singular with the converged eigenvalue if the "
      "full matrix is assembled and the derivative of eigenvalue with respect to the solution "
      "vector is not considered.");

  params.addPrivateParam<bool>("_use_eigen_value", true);

  params.addParam<Real>("initial_eigenvalue", 1, "Initial eigenvalue");
  params.addParam<PostprocessorName>(
      "normalization", "Postprocessor evaluating norm of eigenvector for normalization");
  params.addParam<Real>("normal_factor",
                        "Normalize eigenvector to make a defined norm equal to this factor");

  params.addParam<bool>("auto_initialization",
                        true,
                        "If true, we will set an initial eigen vector in moose, otherwise EPS "
                        "solver will initialize eigen vector");

  params.addParamNamesToGroup("matrix_free precond_matrix_free constant_matrices "
                              "precond_matrix_includes_eigen",
                              "Matrix and Matrix-Free");
  params.addParamNamesToGroup("initial_eigenvalue auto_initialization",
                              "Eigenvector and eigenvalue initialization");
  params.addParamNamesToGroup("normalization normal_factor", "Solution normalization");

  // If Newton and Inverse Power is combined in SLEPc side
  params.addPrivateParam<bool>("_newton_inverse_power", false);

// Add slepc options and eigen problems
#ifdef LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::getSlepcValidParams(params);

  params += Moose::SlepcSupport::getSlepcEigenProblemValidParams();
#endif
  return params;
}

EigenProblemSolve::EigenProblemSolve(Executioner & ex)
  : FEProblemSolve(ex),
    _eigen_problem(*getCheckedPointerParam<EigenProblem *>(
        "_eigen_problem", "This might happen if you don't have a mesh")),
    _normalization(isParamValid("normalization") ? &getPostprocessorValue("normalization")
                                                 : nullptr)
{
// Extract and store SLEPc options
#ifdef LIBMESH_HAVE_SLEPC
  mooseAssert(_problem.numSolverSystems() == 1,
              "The Eigenvalue executioner only currently supports a single solver system.");

  const auto & params = ex.parameters();
  Moose::SlepcSupport::storeSolveType(_eigen_problem, params);

  Moose::SlepcSupport::setEigenProblemSolverParams(_eigen_problem, params);
  _eigen_problem.setEigenproblemType(
      _eigen_problem.solverParams(/*solver_sys_num=*/0)._eigen_problem_type);

  // pass two control parameters to eigen problem
  _eigen_problem.solverParams(/*solver_sys_num=*/0)._free_power_iterations =
      getParam<unsigned int>("free_power_iterations");
  _eigen_problem.solverParams(/*solver_sys_num=*/0)._extra_power_iterations =
      getParam<unsigned int>("extra_power_iterations");

  if (!isParamValid("normalization") && isParamValid("normal_factor"))
    paramError("normal_factor",
               "Cannot set scaling factor without defining normalization postprocessor.");

  if (isParamValid("normalization"))
  {
    const auto & normpp = getParam<PostprocessorName>("normalization");
    if (isParamValid("normal_factor"))
      _eigen_problem.setNormalization(normpp, getParam<Real>("normal_factor"));
    else
      _eigen_problem.setNormalization(normpp);
  }

  _eigen_problem.setInitialEigenvalue(getParam<Real>("initial_eigenvalue"));

  // Set a flag to nonlinear eigen system
  _eigen_problem.getNonlinearEigenSystem(/*nl_sys_num=*/0)
      .precondMatrixIncludesEigenKernels(getParam<bool>("precond_matrix_includes_eigen"));
#else
  mooseError("SLEPc is required to use Eigenvalue executioner, please use '--download-slepc in "
             "PETSc configuration'");
#endif
  // SLEPc older than 3.13.0 can not take initial guess from moose
  // It may generate converge issues
#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
  mooseError(
      "Please use SLEPc-3.13.0 or higher. Old versions of SLEPc likely produce bad convergence");
#endif

  // To avoid petsc unused option warnings, ensure we do not set irrelevant options.
  Moose::PetscSupport::dontAddLinearConvergedReason(_problem);
  Moose::PetscSupport::dontAddNonlinearConvergedReason(_problem);
  Moose::PetscSupport::dontAddPetscFlag("-mat_mffd_type", _problem.getPetscOptions());
  Moose::PetscSupport::dontAddPetscFlag("-st_ksp_atol", _problem.getPetscOptions());
  Moose::PetscSupport::dontAddPetscFlag("-st_ksp_max_it", _problem.getPetscOptions());
  Moose::PetscSupport::dontAddPetscFlag("-st_ksp_rtol", _problem.getPetscOptions());
  if (!_eigen_problem.solverParams()._eigen_matrix_free)
    Moose::PetscSupport::dontAddPetscFlag("-snes_mf_operator", _problem.getPetscOptions());
}

void
EigenProblemSolve::initialSetup()
{
  if (isParamValid("normalization"))
  {
    const auto & normpp = getParam<PostprocessorName>("normalization");
    const auto & exec = _eigen_problem.getUserObject<UserObject>(normpp).getExecuteOnEnum();
    if (!exec.isValueSet(EXEC_LINEAR))
      mooseError("Normalization postprocessor ", normpp, " requires execute_on = 'linear'");
  }

#ifdef LIBMESH_HAVE_SLEPC
  // Options need to be setup once only
  if (!_eigen_problem.petscOptionsInserted())
  {
    // Parent application has the default data base
    if (!_app.isUltimateMaster())
      LibmeshPetscCall(PetscOptionsPush(_eigen_problem.petscOptionsDatabase()));
    Moose::SlepcSupport::slepcSetOptions(
        _eigen_problem, _eigen_problem.solverParams(/*eigen_sys_num=*/0), _pars);
    if (!_app.isUltimateMaster())
      LibmeshPetscCall(PetscOptionsPop());
    _eigen_problem.petscOptionsInserted() = true;
  }
#endif
}
