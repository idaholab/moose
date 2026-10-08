//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEDomainLFDivKernel.h"

registerMooseObject("MooseApp", MFEMVectorFEDomainLFDivKernel);

InputParameters
MFEMVectorFEDomainLFDivKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(f, \\vec\\nabla \\cdot \\vec v)_\\Omega$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMVectorFEDomainLFDivKernel::MFEMVectorFEDomainLFDivKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::LinearFormIntegrator *
MFEMVectorFEDomainLFDivKernel::createLFIntegrator()
{
  return new mfem::VectorFEDomainLFDivIntegrator(_coef);
}

#endif
