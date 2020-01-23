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

  params.addPrivateParam<bool>("_use_eigen_value", true);

  params.addParam<PostprocessorName>(
      "normalization", "Postprocessor evaluating norm of eigenvector for normalization");
  params.addParam<Real>(
      "normal_factor", 1.0, "Normalize eigenvector to make a defined norm equal to this factor");

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
        "_eigen_problem", "This might happen if you don't have a mesh")),
    _normalization(isParamValid("normalization") ? &getPostprocessorValue("normalization")
                                                 : nullptr)
{
// Extract and store SLEPc options
#if LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::storeSlepcOptions(_fe_problem, parameters);

  Moose::SlepcSupport::storeSlepcEigenProblemOptions(_eigen_problem, parameters);
  _eigen_problem.setEigenproblemType(_eigen_problem.solverParams()._eigen_problem_type);
#endif

  if (!parameters.isParamValid("normalization") && parameters.isParamSetByUser("normal_factor"))
    paramError("normal_factor",
               "Cannot set scaling factor without defining normalization postprocessor.");
}

void
Eigenvalue::execute()
{
#if LIBMESH_HAVE_SLEPC
#if PETSC_RELEASE_LESS_THAN(3, 12, 0)
  // Make sure the SLEPc options are setup for this app
  Moose::SlepcSupport::slepcSetOptions(_eigen_problem, _pars);
#else
  if (!_eigen_problem.petscOptionsInserted())
  {
    PetscOptionsPush(_eigen_problem.petscOptionsDatabase());
    Moose::SlepcSupport::slepcSetOptions(_eigen_problem, _pars);
    PetscOptionsPop();
    _eigen_problem.petscOptionsInserted() = true;
  }
#endif
#endif

  Steady::execute();
}

void
Eigenvalue::postSolve()
{
#ifdef LIBMESH_HAVE_SLEPC
  if (_normalization)
  {
    Real val = getParam<Real>("normal_factor");

    if (MooseUtils::absoluteFuzzyEqual(*_normalization, 0.0))
      mooseError("Cannot normalize eigenvector by 0");
    else
      val /= *_normalization;

    if (!MooseUtils::absoluteFuzzyEqual(val, 1.0))
    {
      _eigen_problem.scaleEigenvector(val);
      // update all aux variables and user objects
      for (const ExecFlagType & flag : _app.getExecuteOnEnum().items())
        _problem.execute(flag);
    }
  }
#endif
}
