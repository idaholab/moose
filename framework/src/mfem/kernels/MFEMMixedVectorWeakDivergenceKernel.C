//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedVectorWeakDivergenceKernel.h"

registerMooseObject("MooseApp", MFEMMixedVectorWeakDivergenceKernel);

InputParameters
MFEMMixedVectorWeakDivergenceKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(-k\\vec u, \\nabla v)_\\Omega$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMMixedVectorWeakDivergenceKernel::MFEMMixedVectorWeakDivergenceKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
}

mfem::BilinearFormIntegrator *
MFEMMixedVectorWeakDivergenceKernel::createMBFIntegrator()
{
  return new mfem::MixedVectorWeakDivergenceIntegrator(_coef);
}

#endif
