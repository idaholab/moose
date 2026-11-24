//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EigenvectorBNorm.h"

// MOOSE includes
#include "EigenProblem.h"
#include "NonlinearEigenSystem.h"

registerMooseObject("MooseApp", EigenvectorBNorm);

InputParameters
EigenvectorBNorm::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Reports the B-norm sqrt(phi^T B phi) of the current eigenvector of the nonlinear eigen "
      "system, which the eigen problem computes with the B operator of the solve after each solve "
      "and each time it loads an eigenvector for output.");
  // Meant to be used as the 'normalization' postprocessor, which must execute on linear
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_END};
  return params;
}

EigenvectorBNorm::EigenvectorBNorm(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _eigen_problem(dynamic_cast<EigenProblem *>(&_fe_problem))
{
  if (!dynamic_cast<NonlinearEigenSystem *>(&_sys))
    mooseError("Given system is not a NonlinearEigenSystem");
  mooseAssert(_eigen_problem, "A nonlinear eigen system belongs to an eigen problem");

#ifdef LIBMESH_HAVE_SLEPC
  static_cast<EigenProblem &>(_fe_problem).needEigenvectorBNorm();
#endif
}

Real
EigenvectorBNorm::getValue() const
{
#ifdef LIBMESH_HAVE_SLEPC
  return _eigen_problem->eigenvectorBNorm();
#else
  return 0;
#endif
}
