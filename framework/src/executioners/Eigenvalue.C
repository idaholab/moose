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

// MOOSE includes
#include "Eigenvalue.h"
#include "EigenProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearEigenSystem.h"
#include "SlepcSupport.h"

template <>
InputParameters
validParams<Eigenvalue>()
{
  InputParameters params = validParams<Steady>();

  params.addClassDescription("Eigenvalue solves a standard/generalized eigenvaue problem");

  params.addPrivateParam<bool>("_use_eigen_value", true);

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
  Moose::SlepcSupport::storeSlepcOptions(_fe_problem, _pars);

  Moose::SlepcSupport::storeSlepcEigenProblemOptions(_eigen_problem, parameters);
  _eigen_problem.setEigenproblemType(_eigen_problem.solverParams()._eigen_problem_type);
#endif
}

void
Eigenvalue::execute()
{
#if LIBMESH_HAVE_SLEPC
  // Make sure the SLEPc options are setup for this app
  Moose::SlepcSupport::slepcSetOptions(_eigen_problem, _pars);
#endif
  Steady::execute();
}
