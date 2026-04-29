//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDivDivKernel.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", DivDivKernel);

namespace Moose::MFEM
{
InputParameters
DivDivKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form "
      "$(k\\vec\\nabla \\cdot \\vec u, \\vec\\nabla \\cdot \\vec v)_\\Omega$ "
      "arising from the weak form of the grad-div operator "
      "$-\\vec\\nabla \\left( k \\vec\\nabla \\cdot \\vec u \\right)$.");

  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of property k to multiply the integrator by");

  return params;
}

DivDivKernel::DivDivKernel(const InputParameters & parameters)
  : Kernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
DivDivKernel::createBFIntegrator()
{
  return new mfem::DivDivIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
