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

registerMooseMFEMObject("MooseApp", DomainLFKernel);

namespace Moose::MFEM
{
InputParameters
DomainLFKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient $f$.");
  return params;
}

DomainLFKernel::DomainLFKernel(const InputParameters & parameters)
  : Kernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::LinearFormIntegrator *
DomainLFKernel::createLFIntegrator()
{
  return new mfem::DomainLFIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
