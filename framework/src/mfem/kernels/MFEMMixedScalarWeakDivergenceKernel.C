//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedScalarWeakDivergenceKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMixedScalarWeakDivergenceKernel);

InputParameters
MFEMMixedScalarWeakDivergenceKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k\\vec\\nabla u, \\vec\\nabla v)_\\Omega$ "
                             "arising from the weak form of the Laplacian operator "
                             "$- \\vec\\nabla \\cdot \\left( k \\vec \\nabla u \\right)$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property for MixedScalarWeakDivergence coefficient k.");
  return params;
}

MFEMMixedScalarWeakDivergenceKernel::MFEMMixedScalarWeakDivergenceKernel(
    const InputParameters & parameters)
  : MFEMKernel(parameters), _coef(getScalarCoefficient("coefficient"))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
  // declares GradientGridFunctionCoefficient
  getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
      name(), getMFEMProblem().getProblemData().gridfunctions.Get(_test_var_name));
}

mfem::BilinearFormIntegrator *
MFEMMixedScalarWeakDivergenceKernel::createBFIntegrator()
{
  mfem::VectorCoefficient & vec_coef =
      getMFEMProblem().getCoefficients().getVectorCoefficient(name());
  _product_coef = new mfem::ScalarVectorProductCoefficient(_coef, vec_coef);
  return new mfem::MixedScalarWeakDivergenceIntegrator(*_product_coef);
}

#endif
