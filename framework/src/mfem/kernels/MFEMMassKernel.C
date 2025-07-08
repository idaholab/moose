//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMassKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMassKernel);

InputParameters
MFEMMassKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k u, v)_\\Omega$ "
                             "arising from the weak form of the mass operator "
                             "$ku$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property for the mass coefficient k.");
  params.addParam<MFEMScalarCoefficientName>("coefficient_imag",
                                             "Name of property for the imaginary part of the mass coefficient k.");

  return params;
}

MFEMMassKernel::MFEMMassKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient")))
    // If the imaginary coefficient is not provided, we pick the real one since the variable needs to be initialized, but it won't be used
    _coef_imag_name(isParamValid("coefficient_imag") ? getParam<MFEMScalarCoefficientName>("coefficient_imag") : _coef_name),
    _coef_imag(getScalarCoefficient(_coef_imag_name))
{
}

std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMMassKernel::createBFIntegrator()
{
  return std::make_pair(new mfem::MassIntegrator(_coef), isParamValid("coefficient_imag") ? new mfem::MassIntegrator(_coef_imag)
                                                                                                  : nullptr);
}

#endif
