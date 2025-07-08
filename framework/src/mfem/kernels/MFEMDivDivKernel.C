//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDivDivKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDivDivKernel);

InputParameters
MFEMDivDivKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form "
      "$(k\\vec\\nabla \\cdot \\vec u, \\vec\\nabla \\cdot \\vec v)_\\Omega$ "
      "arising from the weak form of the grad-div operator "
      "$-\\vec\\nabla \\left( k \\vec\\nabla \\cdot \\vec u \\right)$.");

  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property k to multiply the integrator by");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient_imag",
      "Name of the imaginary part of the property k to multiply the Laplacian by");

  return params;
}

MFEMDivDivKernel::MFEMDivDivKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient"))),
    // If the imaginary coefficient is not provided, we pick the real one since the variable needs
    // to be initialized, but it won't be used
    _coef_imag(getScalarCoefficient(isParamValid("coefficient_imag")
                                        ? getParam<MFEMScalarCoefficientName>("coefficient_imag")
                                        : getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMDivDivKernel::createBFIntegrator()
{
  return std::make_pair(new mfem::DivDivIntegrator(_coef),
                        isParamValid("coefficient_imag") ? new mfem::DivDivIntegrator(_coef_imag)
                                                         : nullptr);
}

#endif
