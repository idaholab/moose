//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedVectorGradientKernel.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", MixedVectorGradientKernel);

namespace Moose::MFEM
{
InputParameters
MixedVectorGradientKernel::validParams()
{
  InputParameters params = MixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k\\vec\\nabla u, \\vec v)_\\Omega$ "
      "arising from the weak form of the gradient operator "
      "$k\\vec \\nabla u$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of property k to use.");
  return params;
}

MixedVectorGradientKernel::MixedVectorGradientKernel(const InputParameters & parameters)
  : MixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
}

mfem::BilinearFormIntegrator *
MixedVectorGradientKernel::createMBFIntegrator()
{
  return new mfem::MixedVectorGradientIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
