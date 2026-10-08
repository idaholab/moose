//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedScalarWeakDerivativeKernel.h"

registerMooseObject("MooseApp", MFEMMixedScalarWeakDerivativeKernel);

InputParameters
MFEMMixedScalarWeakDerivativeKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form $-\\left(k u, "
      "\\frac{\\partial v}{\\partial x}\\right)_\\Omega$ arising from the weak form of the "
      "operator $\\frac{\\partial}{\\partial x}\\left(k u\\right)$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMMixedScalarWeakDerivativeKernel::MFEMMixedScalarWeakDerivativeKernel(
    const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedScalarWeakDerivativeKernel::createMBFIntegrator()
{
  return new mfem::MixedScalarWeakDerivativeIntegrator(_coef);
}

#endif
