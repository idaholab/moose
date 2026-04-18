//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEDivergenceKernel.h"

registerMooseMFEMObject("MooseApp", VectorFEDivergenceKernel);

namespace Moose::MFEM
{
InputParameters
VectorFEDivergenceKernel::validParams()
{
  InputParameters params = MixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k \\vec \\nabla \\cdot \\vec u, v)_\\Omega$ arising from the weak form "
      "of the divergence operator $k \\vec \\nabla \\cdot \\vec u$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of property k to use.");
  return params;
}

VectorFEDivergenceKernel::VectorFEDivergenceKernel(const InputParameters & parameters)
  : MixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
VectorFEDivergenceKernel::createMBFIntegrator()
{
  return new mfem::VectorFEDivergenceIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
