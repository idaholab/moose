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

registerMooseMFEMObject("MooseApp", MassKernel);

namespace Moose::MFEM
{
InputParameters
MassKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k u, v)_\\Omega$ "
                             "arising from the weak form of the mass operator "
                             "$ku$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of property for the mass coefficient k.");
  return params;
}

MassKernel::MassKernel(const InputParameters & parameters)
  : Kernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
MassKernel::createBFIntegrator()
{
  return new mfem::MassIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
