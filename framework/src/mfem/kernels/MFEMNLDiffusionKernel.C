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

InputParameters
MFEMNLDiffusionKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator for integrating the non-linear action"
                             "$(k(u)\\vec\\nabla v, \\vec\\nabla v)_\Omega$"
                             "Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$((k(u)\\vec\\nabla v, \\vec\\nabla v)_\Omega + (k'(u) v, "
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
  // declares GradientGridFunctionCoefficient
  getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
      name(), getMFEMProblem().getProblemData().gridfunctions.Get(_test_var_name));
}

mfem::BilinearFormIntegrator *
MFEMNLDiffusionKernel::createBFIntegrator()
{
  _sum = new mfem::SumIntegrator;
  _sum->AddIntegrator(new mfem::DiffusionIntegrator(_coef));
  mfem::VectorCoefficient & vec_coef =
      getMFEMProblem().getCoefficients().getVectorCoefficient(name());
  _one = new mfem::ConstantCoefficient(1.0);
  _product_coef_jac = new mfem::ScalarVectorProductCoefficient(*_one, vec_coef);
  _sum->AddIntegrator(new mfem::MixedScalarWeakDivergenceIntegrator(*_product_coef_jac));
  return _sum;
}

mfem::LinearFormIntegrator *
MFEMNLDiffusionKernel::createNLAIntegrator()
{
  mfem::VectorCoefficient & vec_coef =
      getMFEMProblem().getCoefficients().getVectorCoefficient(name());
  _product_coef_res = new mfem::ScalarVectorProductCoefficient(_coef, vec_coef);
  return new mfem::DomainLFGradIntegrator(*_product_coef_res);
}

#endif
