//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDerivativeKernel.h"

registerMooseObject("MooseApp", MFEMDerivativeKernel);

InputParameters
MFEMDerivativeKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the mixed bilinear "
                             "form $(k \\partial_i u, v)_\\Omega$ arising from the weak form of "
                             "the operator $k \\partial_i u$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 3",
      "Index i of the spatial direction to differentiate along (0 for x, 1 for y, 2 for z).");
  return params;
}

MFEMDerivativeKernel::MFEMDerivativeKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters),
    _coef(getScalarCoefficient("coefficient")),
    _component(getParam<unsigned int>("component"))
{
}

mfem::BilinearFormIntegrator *
MFEMDerivativeKernel::createMBFIntegrator()
{
  return new mfem::DerivativeIntegrator(_coef, _component);
}

#endif
