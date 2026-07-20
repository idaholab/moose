//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGDiffusionKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDGDiffusionKernel);

InputParameters
MFEMDGDiffusionKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k\\vec\\nabla u, \\vec\\nabla v)_\\Omega$ "
                             "arising from the weak form of the Laplacian operator "
                             "$- \\vec\\nabla \\cdot \\left( k \\vec \\nabla u \\right)$.");
  params.addParam<mfem::real_t>("coef", 1.0, "Name of property for diffusion coefficient k");
  params.addParam<mfem::real_t>("sigma", -1.0, "Symmetry parameter. Typically +/- 1.0");
  params.addParam<mfem::real_t>(
      "kappa", "Penalty parameter. Should be non-negative. Will default to (order+1)^2");
  return params;
}

MFEMDGDiffusionKernel::MFEMDGDiffusionKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _fe_order(getMFEMProblem().getGridFunction(_test_var_name)->ParFESpace()->FEColl()->GetOrder()),
    _coef(getScalarCoefficient("coefficient")),
    _sigma(getParam<mfem::real_t>("sigma")),
    _kappa((isParamSetByUser("kappa")) ? getParam<mfem::real_t>("kappa")
                                       : (_fe_order + 1) * (_fe_order + 1))
{
}

mfem::BilinearFormIntegrator *
MFEMDGDiffusionKernel::createBFIntegrator()
{
  return new mfem::DGDiffusionIntegrator(_coef, _sigma, _kappa);
}

#endif
