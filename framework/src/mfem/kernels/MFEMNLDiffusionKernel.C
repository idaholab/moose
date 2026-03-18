//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNLDiffusionKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMNLDiffusionKernel);

MFEMNLDiffusionIntegrator::MFEMNLDiffusionIntegrator(mfem::Coefficient & q,
                                                     const mfem::GridFunction * gf,
                                                     const mfem::IntegrationRule * ir)
  : _grad_trial(gf),
    _diffusion_integ(q, ir),
    _product_coef_jac(_one, _grad_trial),
    _weak_div_integ(_product_coef_jac)
{
  _sum.AddIntegrator(&_diffusion_integ);
  _sum.AddIntegrator(&_weak_div_integ);
}

void
MFEMNLDiffusionIntegrator::AssembleElementVector(const mfem::FiniteElement & el,
                                                 mfem::ElementTransformation & Tr,
                                                 const mfem::Vector & elfun,
                                                 mfem::Vector & elvect)
{
  _diffusion_integ.AssembleElementVector(el, Tr, elfun, elvect);
}

void
MFEMNLDiffusionIntegrator::AssembleElementGrad(const mfem::FiniteElement & el,
                                               mfem::ElementTransformation & Tr,
                                               const mfem::Vector & elfun,
                                               mfem::DenseMatrix & elmat)
{
  _sum.AssembleElementGrad(el, Tr, elfun, elmat);
}

InputParameters
MFEMNLDiffusionKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator for integrating the non-linear action"
                             "$(k(u)\\vec\\nabla u, \\vec\\nabla v)_\\Omega$"
                             "Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k(u)\\vec\\nable u, \\vec\\nabla v)_\\Omega + (k'(u) u, "
                             "\\vec\\nabla u \\vec\\nabla v)_\\Omega$ "
                             "The above terms arises from the weak form of the non-linear operator "
                             "$- \\vec\\nabla \\cdot ( k(u) \\vec\\nabla u)$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property for MixedScalarWeakDivergence coefficient k.");
  return params;
}

MFEMNLDiffusionKernel::MFEMNLDiffusionKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _coef(getScalarCoefficient("coefficient"))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
}

mfem::NonlinearFormIntegrator *
MFEMNLDiffusionKernel::createNLIntegrator()
{
  return new MFEMNLDiffusionIntegrator(
      _coef, getMFEMProblem().getProblemData().gridfunctions.Get(_test_var_name));
}

#endif
