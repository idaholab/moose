//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedGradGradKernel.h"

registerMooseMFEMObject("MooseApp", MixedGradGradKernel);

namespace Moose::MFEM
{
InputParameters
MixedGradGradKernel::validParams()
{
  InputParameters params = MixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k\\vec\\nabla u, \\vec\\nabla v)_\\Omega$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of property k to use.");
  return params;
}

MixedGradGradKernel::MixedGradGradKernel(const InputParameters & parameters)
  : MixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
}

mfem::BilinearFormIntegrator *
MixedGradGradKernel::createMBFIntegrator()
{
  return new mfem::MixedGradGradIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
