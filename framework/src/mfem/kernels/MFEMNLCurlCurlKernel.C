//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNLCurlCurlKernel.h"
#include "MFEMProblem.h"
#include "NLCurlCurlIntegrator.h"

registerMooseObject("MooseApp", MFEMNLCurlCurlKernel);

InputParameters
MFEMNLCurlCurlKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the nonlinear form "
      "$(k(|\\vec \\nabla \\times \\vec u|) \\vec \\nabla \\times \\vec u, \\vec \\nabla \\times "
      "\\vec v)_\\Omega$ "
      "arising from the weak form of the non-linear operator "
      "$\\vec \\nabla \\times (k(|\\vec \\nabla \\times \\vec u|) \\vec \\nabla \\times \\vec "
      "u)$.");
  params.addParam<MFEMScalarCoefficientName>(
      "k_coefficient",
      "1.",
      "Name of the nonlinear coefficient $k(|\\vec\\nabla \\times \\vec u|)$.");
  params.addParam<MFEMScalarCoefficientName>(
      "curlu_dk_dcurlu_coefficient",
      "0.",
      "Name of the coefficient representing "
      "$|\\vec \\nabla \\times \\vec u| \\partial k(|\\nabla "
      "\\times \\vec u|)/\\partial |\\nabla \\times \\vec u|$");
  params.addParam<mfem::real_t>(
      "curlu_zero_tol",
      1e-32,
      "Tolerance used for normalizing the curl vector when forming the Jacobian.");
  return params;
}

MFEMNLCurlCurlKernel::MFEMNLCurlCurlKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _k_coef(getScalarCoefficient("k_coefficient")),
    _curlu_dk_dcurlu_coef(getScalarCoefficient("curlu_dk_dcurlu_coefficient")),
    _curlu_vec_coef(getVectorCoefficientByName(getTrialVariableName() + "_curl")),
    _curlu_zero_tol(getParam<mfem::real_t>("curlu_zero_tol"))
{
}

mfem::NonlinearFormIntegrator *
MFEMNLCurlCurlKernel::createNLIntegrator()
{
  return new Moose::MFEM::NLCurlCurlIntegrator(
      _k_coef, _curlu_dk_dcurlu_coef, _curlu_vec_coef, _curlu_zero_tol);
}

#endif
