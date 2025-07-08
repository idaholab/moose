//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEWeakDivergenceKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFEWeakDivergenceKernel);

InputParameters
MFEMVectorFEWeakDivergenceKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(-k\\vec u, \\vec\\nabla v)_\\Omega$ "
      "arising from the weak form of the divergence operator "
      "$\\vec \\nabla \\cdot (k\\vec u)$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient_imag", "Name of the imaginary part of the property k to use.");
  return params;
}

MFEMVectorFEWeakDivergenceKernel::MFEMVectorFEWeakDivergenceKernel(
    const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient"))),
    // If the imaginary coefficient is not provided, we pick the real one since the variable needs
    // to be initialized, but it won't be used
    _coef_imag(getScalarCoefficient(isParamValid("coefficient_imag")
                                        ? getParam<MFEMScalarCoefficientName>("coefficient_imag")
                                        : getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMVectorFEWeakDivergenceKernel::createBFIntegrator()
{
  return std::make_pair(new mfem::VectorFEWeakDivergenceIntegrator(_coef),
                        isParamValid("coefficient_imag")
                            ? new mfem::VectorFEWeakDivergenceIntegrator(_coef_imag)
                            : nullptr);
}

#endif
