//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEDomainLFCurlKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFEDomainLFCurlKernel);

InputParameters
MFEMVectorFEDomainLFCurlKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\nabla \\times \\vec v)_\\Omega$ "
                             "arising from the weak form of the forcing term "
                             "$\\nabla \\times \\vec f$.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "The name of the vector coefficient f");
  return params;
}

MFEMVectorFEDomainLFCurlKernel::MFEMVectorFEDomainLFCurlKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

mfem::LinearFormIntegrator *
MFEMVectorFEDomainLFCurlKernel::createLFIntegrator()
{
  return new mfem::VectorFEDomainLFCurlIntegrator(_vec_coef);
}

#endif
