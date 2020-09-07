//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Eigenvalue.h"
#include "EigenProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearEigenSystem.h"
#include "SlepcSupport.h"

registerMooseObject("MooseApp", Eigenvalue);

defineLegacyParams(Eigenvalue);

InputParameters
Eigenvalue::validParams()
{
  InputParameters params = Steady::validParams();

  params.addClassDescription("Eigenvalue solves a standard/generalized eigenvaue problem");

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

  params.addParam<bool>("precond_matrix_includes_eigen",
                        false,
                        "Whether or not to include eigen kernels in the preconditioning matrix. "
                        "If true, the preconditioning matrix will include eigen kernels.");

  params.addPrivateParam<bool>("_use_eigen_value", true);

  params.addParam<PostprocessorName>(
      "normalization", "Postprocessor evaluating norm of eigenvector for normalization");
  params.addParam<Real>("normal_factor",
                        "Normalize eigenvector to make a defined norm equal to this factor");

// Add slepc options and eigen problems
#ifdef LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::getSlepcValidParams(params);

  params += Moose::SlepcSupport::getSlepcEigenProblemValidParams();
#endif
  return params;
}

Eigenvalue::Eigenvalue(const InputParameters & parameters)
  : Steady(parameters),
    _eigen_problem(*getCheckedPointerParam<EigenProblem *>(
        "_eigen_problem", "This might happen if you don't have a mesh"))
{
// Extract and store SLEPc options
#if LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::storeSlepcOptions(_fe_problem, parameters);

  Moose::SlepcSupport::storeSlepcEigenProblemOptions(_eigen_problem, parameters);
  _eigen_problem.setEigenproblemType(_eigen_problem.solverParams()._eigen_problem_type);
#endif

  if (!isParamValid("normalization") && isParamValid("normal_factor"))
    paramError("normal_factor",
               "Cannot set scaling factor without defining normalization postprocessor.");

  if (isParamValid("normalization"))
  {
    auto normpp = getParam<PostprocessorName>("normalization");
    if (isParamValid("normal_factor"))
      _eigen_problem.setNormalization(normpp, getParam<Real>("normal_factor"));
    else
      _eigen_problem.setNormalization(normpp);
  }
}

void
Eigenvalue::init()
{
#if LIBMESH_HAVE_SLEPC
  // Set a flag to nonlinear eigen system
  _eigen_problem.getNonlinearEigenSystem().precondMatrixIncludesEigenKernels(
      getParam<bool>("precond_matrix_includes_eigen"));
#endif
  if (isParamValid("normalization"))
  {
    auto normpp = getParam<PostprocessorName>("normalization");
    const auto & exec = _problem.getUserObject<UserObject>(normpp).getExecuteOnEnum();
    if (!exec.contains(EXEC_LINEAR))
      mooseError("Normalization postprocessor ", normpp, " requires execute_on = 'linear'");
  }
  Steady::init();
}

void
Eigenvalue::execute()
{
#if LIBMESH_HAVE_SLEPC
#if PETSC_RELEASE_LESS_THAN(3, 12, 0)
  // Make sure the SLEPc options are setup for this app
  Moose::SlepcSupport::slepcSetOptions(_eigen_problem, _pars);
#else
  // Options need to be setup once only
  if (!_eigen_problem.petscOptionsInserted())
  {
    // Master app has the default data base
    if (!_app.isUltimateMaster())
      PetscOptionsPush(_eigen_problem.petscOptionsDatabase());

    Moose::SlepcSupport::slepcSetOptions(_eigen_problem, _pars);

    if (!_app.isUltimateMaster())
      PetscOptionsPop();

    _eigen_problem.petscOptionsInserted() = true;
  }
#endif
#endif

  Steady::execute();
}
