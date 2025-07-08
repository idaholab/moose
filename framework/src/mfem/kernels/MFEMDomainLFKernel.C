//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDomainLFKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDomainLFKernel);

InputParameters
MFEMDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient $f$.");
  params.addParam<std::string>("coefficient_imag", "Name of the imaginary part of the scalar coefficient $f$.");
  return params;
}

MFEMDomainLFKernel::MFEMDomainLFKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient"))),
    // If the imaginary coefficient is not provided, we pick the real one since the variable needs to be initialized, but it won't be used
    _coef_imag_name(isParamValid("coefficient_imag") ? getParam<std::string>("coefficient_imag") : getParam<MFEMScalarCoefficientName>("coefficient")),
    _coef_imag(getScalarCoefficient(_coef_imag_name))
{
}

std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *>
MFEMDomainLFKernel::createLFIntegrator()
{
  return std::make_pair(new mfem::DomainLFIntegrator(_coef), isParamValid("coefficient_imag") ? new mfem::DomainLFIntegrator(_coef_imag)
                                                                                                  : nullptr);
}

#endif
