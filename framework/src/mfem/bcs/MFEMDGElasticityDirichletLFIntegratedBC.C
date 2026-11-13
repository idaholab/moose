//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGElasticityDirichletLFIntegratedBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDGElasticityDirichletLFIntegratedBC);

InputParameters
MFEMDGElasticityDirichletLFIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription(
      "Adds the boundary face integrator to an MFEM problem for the linear form "
      "$\\alpha \\langle \\vec u_D, \\sigma(\\vec v) \\cdot \\hat n \\rangle + \\kappa \\langle "
      "h^{-1} (\\lambda + 2 \\mu) \\vec u_D, \\vec v \\rangle$, which weakly imposes the "
      "Dirichlet data $\\vec u_D$ in DG discretizations of isotropic linear elasticity.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Name of the Dirichlet data u_D to use.");
  params.addParam<MFEMScalarCoefficientName>(
      "lambda", "1.", "Name of MFEM Lame constant lambda to use.");
  params.addParam<MFEMScalarCoefficientName>("mu", "1.", "Name of MFEM Lame constant mu to use.");
  params.addParam<mfem::real_t>(
      "alpha", -1.0, "Symmetry parameter: -1 for SIPG, 0 for IIPG and 1 for NIPG.");
  params.addParam<mfem::real_t>(
      "kappa", "Penalty parameter. Should be non-negative. Will default to (order+1)^2");
  return params;
}

MFEMDGElasticityDirichletLFIntegratedBC::MFEMDGElasticityDirichletLFIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _fe_order(getMFEMProblem().getGridFunction(_test_var_name)->ParFESpace()->FEColl()->GetOrder()),
    _vec_coef(getVectorCoefficient("vector_coefficient")),
    _lambda(getScalarCoefficient("lambda")),
    _mu(getScalarCoefficient("mu")),
    _alpha(getParam<mfem::real_t>("alpha")),
    _kappa((isParamSetByUser("kappa")) ? getParam<mfem::real_t>("kappa")
                                       : (_fe_order + 1) * (_fe_order + 1))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMDGElasticityDirichletLFIntegratedBC::createLFIntegrator()
{
  return new mfem::DGElasticityDirichletLFIntegrator(_vec_coef, _lambda, _mu, _alpha, _kappa);
}

#endif
