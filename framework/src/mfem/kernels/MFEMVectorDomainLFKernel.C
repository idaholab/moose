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

registerMooseMFEMObject("MooseApp", VectorDomainLFKernel);

namespace Moose::MFEM
{
InputParameters
VectorDomainLFKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_\\Omega$ "
                             "arising from the weak form of the forcing term $\\vec f$.");
  params.addParam<Moose::MFEM::VectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Name of body force density f.");
  return params;
}

VectorDomainLFKernel::VectorDomainLFKernel(const InputParameters & parameters)
  : Kernel(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

mfem::LinearFormIntegrator *
VectorDomainLFKernel::createLFIntegrator()
{
  return new mfem::VectorDomainLFIntegrator(_vec_coef);
}

} // namespace Moose::MFEM
#endif
