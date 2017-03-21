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

#include "Eigenvalues.h"
#include "libmesh/libmesh_config.h"

template <>
InputParameters
validParams<Eigenvalues>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  return params;
}

Eigenvalues::Eigenvalues(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
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
}

void
Eigenvalues::execute()
{
#if LIBMESH_HAVE_SLEPC
  const std::vector<std::pair<Real, Real>> & eigenvalues = _nl_eigen->getAllConvergedEigenvalues();
  unsigned int n_converged_eigenvalues = eigenvalues.size();
  _eigen_values_real.resize(n_converged_eigenvalues);
  _eigen_values_imag.resize(n_converged_eigenvalues);
  for (unsigned int n = 0; n < n_converged_eigenvalues; n++)
  {
    _eigen_values_real[n] = eigenvalues[n].first;
    _eigen_values_imag[n] = eigenvalues[n].second;
  }
#else
  _eigen_values_real.clear();
  _eigen_values_imag.clear();
#endif
}
