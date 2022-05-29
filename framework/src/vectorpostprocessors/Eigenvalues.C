//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Eigenvalues.h"

// MOOSE includes
#include "NonlinearEigenSystem.h"

#include "libmesh/libmesh_config.h"

#include <complex>

registerMooseObject("MooseApp", Eigenvalues);

InputParameters
Eigenvalues::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Returns the Eigen values from the nonlinear Eigen system.");
  params.addParam<bool>("inverse_eigenvalue", false, "True to evaluate the inverse of eigenvalues");
  return params;
}

Eigenvalues::Eigenvalues(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _inverse(getParam<bool>("inverse_eigenvalue")),
    _eigen_values_real(declareVector("eigen_values_real")),
    _eigen_values_imag(declareVector("eigen_values_imag")),
    _nl_eigen(dynamic_cast<NonlinearEigenSystem *>(&_fe_problem.getNonlinearSystemBase()))
{
  if (!_nl_eigen)
    mooseError("Given system is not a NonlinearEigenSystem \n");
}

void
Eigenvalues::initialize()
{
  _eigen_values_real.clear();
  _eigen_values_imag.clear();
}

void
Eigenvalues::execute()
{
#ifdef LIBMESH_HAVE_SLEPC
  const std::vector<std::pair<Real, Real>> & eigenvalues = _nl_eigen->getAllConvergedEigenvalues();
  unsigned int n_converged_eigenvalues = eigenvalues.size();
  _eigen_values_real.resize(n_converged_eigenvalues);
  _eigen_values_imag.resize(n_converged_eigenvalues);
  for (unsigned int n = 0; n < n_converged_eigenvalues; n++)
  {
    std::complex<Real> e(eigenvalues[n].first, eigenvalues[n].second);
    std::complex<Real> inv = _inverse ? 1. / e : e;
    _eigen_values_real[n] = inv.real();
    _eigen_values_imag[n] = inv.imag();
  }
#else
  _eigen_values_real.clear();
  _eigen_values_imag.clear();
#endif
}
