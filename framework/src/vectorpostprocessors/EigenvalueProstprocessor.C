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

#include "EigenvaluePostprocessor.h"

template<>
InputParameters validParams<EigenvaluePostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  return params;
}

EigenvaluePostprocessor::EigenvaluePostprocessor(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    _eigen_values_real(declareVector("eigen_values_real")),
    _eigen_values_imag(declareVector("eigen_values_imag")),
    _nl_eigen(dynamic_cast<NonlinearEigenSystem &>(_fe_problem.getNonlinearSystemBase()))
{}

void
EigenvaluePostprocessor::initialize()
{}

void
EigenvaluePostprocessor::execute()
{
  unsigned int n_converged_eigenvalues = _nl_eigen.getNumConvergedEigenvalues();
  _eigen_values_real.clear();
  _eigen_values_real.reserve(n_converged_eigenvalues);
  _eigen_values_imag.clear();
  _eigen_values_imag.reserve(n_converged_eigenvalues);
  for (unsigned int n = 0; n < n_converged_eigenvalues; n++)
  {
    _eigen_values_real.push_back(_nl_eigen.getAllConvergedEigenvalues()[n].first);
    _eigen_values_imag.push_back(_nl_eigen.getAllConvergedEigenvalues()[n].second);
  }
}
