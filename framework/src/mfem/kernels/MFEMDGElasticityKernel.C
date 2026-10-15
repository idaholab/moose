//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGElasticityKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDGElasticityKernel);

InputParameters
MFEMDGElasticityKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the interior face integrator to an MFEM problem for the DG isotropic linear "
      "elasticity face terms $- \\langle \\{ \\sigma(\\vec u) \\cdot \\hat n \\}, [\\vec v] "
      "\\rangle + \\alpha \\langle \\{ \\sigma(\\vec v) \\cdot \\hat n \\}, [\\vec u] \\rangle + "
      "\\kappa \\langle h^{-1} \\{ \\lambda + 2 \\mu \\} [\\vec u], [\\vec v] \\rangle$.");
  params.addParam<MFEMScalarCoefficientName>(
      "lambda", "1.", "Name of MFEM Lame constant lambda to use.");
  params.addParam<MFEMScalarCoefficientName>("mu", "1.", "Name of MFEM Lame constant mu to use.");
  params.addParam<mfem::real_t>(
      "alpha", -1.0, "Symmetry parameter: -1 for SIPG, 0 for IIPG and 1 for NIPG.");
  params.addParam<mfem::real_t>(
      "kappa", "Penalty parameter. Should be non-negative. Will default to (order+1)^2");
  return params;
}

MFEMDGElasticityKernel::MFEMDGElasticityKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _fe_order(getMFEMProblem().getGridFunction(_test_var_name)->ParFESpace()->FEColl()->GetOrder()),
    _lambda(getScalarCoefficient("lambda")),
    _mu(getScalarCoefficient("mu")),
    _alpha(getParam<mfem::real_t>("alpha")),
    _kappa((isParamSetByUser("kappa")) ? getParam<mfem::real_t>("kappa")
                                       : (_fe_order + 1) * (_fe_order + 1))
{
}

mfem::BilinearFormIntegrator *
MFEMDGElasticityKernel::createBFIntegrator()
{
  return new mfem::DGElasticityIntegrator(_lambda, _mu, _alpha, _kappa);
}

#endif
