//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDomainLFGradKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDomainLFGradKernel);

InputParameters
MFEMDomainLFGradKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\grad v)_\\Omega$.");
  params.addParam<MFEMVectorCoefficientName>("vector_coefficient",
                                             "Name of vector coefficient $\\vec f$.");
  return params;
}

MFEMDomainLFGradKernel::MFEMDomainLFGradKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

mfem::LinearFormIntegrator *
MFEMDomainLFGradKernel::createLFIntegrator()
{
  return new mfem::DomainLFGradIntegrator(_vec_coef);
}

#endif
