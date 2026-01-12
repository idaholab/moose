//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDomainNLGradKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDomainNLGradKernel);

InputParameters
MFEMDomainNLGradKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain Grad integrator to an MFEM problem for the generally non-linear form "
                             "$(f(v), grad(v) )_\\Omega$ "
                             "arising from the weak form of the flux term $div(f)$.");
  params.addParam<MFEMVectorCoefficientName>(
      "coefficient", "1.", "Name of Vector coefficient $f$.");
  return params;
}

MFEMDomainNLGradKernel::MFEMDomainNLGradKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _coef(getVectorCoefficient("coefficient"))
{
}

mfem::LinearFormIntegrator *
MFEMDomainNLGradKernel::createNLAIntegrator()
{
  return new mfem::DomainLFGradIntegrator(_coef);
}

#endif
