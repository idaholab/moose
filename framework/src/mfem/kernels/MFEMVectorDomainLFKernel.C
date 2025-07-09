//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorDomainLFKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorDomainLFKernel);

InputParameters
MFEMVectorDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_\\Omega$ "
                             "arising from the weak form of the forcing term $\\vec f$.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Name of body force density f.");
  return params;
}

MFEMVectorDomainLFKernel::MFEMVectorDomainLFKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _vec_coef(getVectorCoefficient(getParam<MFEMVectorCoefficientName>("vector_coefficient")))
{
}

mfem::LinearFormIntegrator *
MFEMVectorDomainLFKernel::createLFIntegrator()
{
  return new mfem::VectorDomainLFIntegrator(_vec_coef);
}

#endif
