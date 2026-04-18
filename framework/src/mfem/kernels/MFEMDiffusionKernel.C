//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDiffusionKernel.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", DiffusionKernel);

namespace Moose::MFEM
{
InputParameters
DiffusionKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k\\vec\\nabla u, \\vec\\nabla v)_\\Omega$ "
                             "arising from the weak form of the Laplacian operator "
                             "$- \\vec\\nabla \\cdot \\left( k \\vec \\nabla u \\right)$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of property for diffusion coefficient k.");
  return params;
}

DiffusionKernel::DiffusionKernel(const InputParameters & parameters)
  : Kernel(parameters), _coef(getScalarCoefficient("coefficient"))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
}

mfem::BilinearFormIntegrator *
DiffusionKernel::createBFIntegrator()
{
  return new mfem::DiffusionIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
