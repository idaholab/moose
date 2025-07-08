//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEDomainLFKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFEDomainLFKernel);

InputParameters
MFEMVectorFEDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_\\Omega$ "
                             "arising from the weak form of the forcing term $\\vec f$.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "The name of the vector coefficient f");
  params.addParam<MFEMVectorCoefficientName>("vector_coefficient_imag",
                                             "The name of the imaginary part of the vector coefficient f");

  return params;
}

MFEMVectorFEDomainLFKernel::MFEMVectorFEDomainLFKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _vec_coef(getVectorCoefficient(getParam<MFEMVectorCoefficientName>("vector_coefficient")))
    // If the imaginary coefficient is not provided, we pick the real one since the variable needs to be initialized, but it won't be used
    _vec_coef_imag_name(isParamValid("vector_coefficient_imag") ? getParam<MFEMVectorCoefficientName>("vector_coefficient_imag")
                                                                 : _vec_coef_name),
    _vec_coef_imag(getVectorCoefficient(_vec_coef_imag_name))
{
}

std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *>
MFEMVectorFEDomainLFKernel::createLFIntegrator()
{
  return std::make_pair(new mfem::VectorFEDomainLFIntegrator(_vec_coef), isParamValid("vector_coefficient_imag") ? new mfem::VectorFEDomainLFIntegrator(_vec_coef_imag)
                                                                                                                      : nullptr);
}

#endif
