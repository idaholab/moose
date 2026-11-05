//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedScalarDerivativeKernel.h"

registerMooseObject("MooseApp", MFEMMixedScalarDerivativeKernel);

InputParameters
MFEMMixedScalarDerivativeKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form $\\left(k "
      "\\frac{\\partial u}{\\partial x}, v\\right)_\\Omega$ arising from the weak form of the "
      "operator $k \\frac{\\partial u}{\\partial x}$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMMixedScalarDerivativeKernel::MFEMMixedScalarDerivativeKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedScalarDerivativeKernel::createMBFIntegrator()
{
  return new mfem::MixedScalarDerivativeIntegrator(_coef);
}

#endif
