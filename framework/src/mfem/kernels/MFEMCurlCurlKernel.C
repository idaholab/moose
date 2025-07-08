//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCurlCurlKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMCurlCurlKernel);

InputParameters
MFEMCurlCurlKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form "
      "$(k\\vec\\nabla \\times \\vec u, \\vec\\nabla \\times \\vec v)_\\Omega$ "
      "arising from the weak form of the curl curl operator "
      "$k\\vec\\nabla \\times \\vec\\nabla \\times \\vec u$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient k to multiply the integrator by.");
  params.addParam<MFEMScalarCoefficientName>(
    "coefficient_imag", "Name of the imaginary part of the scalar coefficient k to multiply the integrator by.");
  return params;
}

MFEMCurlCurlKernel::MFEMCurlCurlKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient"))),
    // If the imaginary coefficient is not provided, we pick the real one since the variable needs to be initialized, but it won't be used
    _coef_imag_name(isParamValid("coefficient_imag") ? getParam<MFEMScalarCoefficientName>("coefficient_imag") : getParam<MFEMScalarCoefficientName>("coefficient")),
    _coef_imag(getScalarCoefficient(_coef_imag_name))
{
}

std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMCurlCurlKernel::createBFIntegrator()
{
  return std::make_pair(new mfem::CurlCurlIntegrator(_coef), isParamValid("coefficient_imag") ? new mfem::CurlCurlIntegrator(_coef_imag)
                                                                                                  : nullptr);
}

#endif
