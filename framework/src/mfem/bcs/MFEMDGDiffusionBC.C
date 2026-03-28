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
  params.addParam<mfem::real_t>("sigma", -1.0, "One of the DG penalty params. Typically +/- 1.0");
  params.addParam<mfem::real_t>(
      "kappa", "One of the DG penalty params. Should be positive. Will default to (order+1)^2");
  return params;
}

MFEMDGDiffusionBC::MFEMDGDiffusionBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _fe_order(getMFEMProblem()
                  .getProblemData()
                  .gridfunctions.Get(_test_var_name)
                  ->ParFESpace()
                  ->FEColl()
                  ->GetOrder()),
    _one(1.0),
    _zero(0.0),
    _sigma(getParam<mfem::real_t>("sigma")),
    _kappa((isParamSetByUser("kappa")) ? getParam<mfem::real_t>("kappa")
                                       : (_fe_order + 1) * (_fe_order + 1))
{
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMDGDiffusionBC::createFaceBFIntegrator()
{
  return new mfem::DGDiffusionIntegrator(_one, _sigma, _kappa);
}

#endif
