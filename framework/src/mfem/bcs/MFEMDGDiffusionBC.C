//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGDiffusionBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDGDiffusionBC);

InputParameters
MFEMDGDiffusionBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Boundary condition for DG Diffusion kernel");
  params.addParam<MFEMScalarCoefficientName>(
      "diffusion_coefficient", "1.0", "Name of property for diffusion coefficient k");
  params.addParam<MFEMScalarCoefficientName>(
      "boundary_coefficient", "0.0", "Name of property for dirichlet coefficient");
  params.addParam<mfem::real_t>("sigma", -1.0, "Symmetry parameter. Typically +/- 1.0");
  params.addParam<mfem::real_t>(
      "kappa", "Penalty parameter. Should be non-negative. Will default to (order+1)^2");
  return params;
}

MFEMDGDiffusionBC::MFEMDGDiffusionBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _fe_order(getMFEMProblem().getGridFunction(_test_var_name)->ParFESpace()->FEColl()->GetOrder()),
    _dcoef(getScalarCoefficient("diffusion_coefficient")),
    _bcoef(getScalarCoefficient("boundary_coefficient")),
    _sigma(getParam<mfem::real_t>("sigma")),
    _kappa((isParamSetByUser("kappa")) ? getParam<mfem::real_t>("kappa")
                                       : (_fe_order + 1) * (_fe_order + 1))
{
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMDGDiffusionBC::createBFIntegrator()
{
  return new mfem::DGDiffusionIntegrator(_dcoef, _sigma, _kappa);
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMDGDiffusionBC::createLFIntegrator()
{
  return new mfem::DGDirichletLFIntegrator(_bcoef, _dcoef, _sigma, _kappa);
}

#endif
