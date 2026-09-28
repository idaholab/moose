//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedCurlKernel.h"

registerMooseObject("MooseApp", MFEMMixedCurlKernel);

InputParameters
MFEMMixedCurlKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form $(k \\vec\\nabla "
      "\\times \\vec u, \\vec v)_\\Omega$ arising from the weak form of the operator $k "
      "\\vec\\nabla \\times \\vec u$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMMixedCurlKernel::MFEMMixedCurlKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedCurlKernel::createMBFIntegrator()
{
  return new mfem::MixedCurlIntegrator(_coef);
}

#endif
