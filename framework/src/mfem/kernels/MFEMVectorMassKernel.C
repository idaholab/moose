//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorMassKernel.h"

registerMooseObject("MooseApp", MFEMVectorMassKernel);

InputParameters
MFEMVectorMassKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form "
      "$(k \\vec u, \\vec v)_\\Omega$, where $\\vec u$ and $\\vec v$ are in vector "
      "$H^1$ or $L^2$ spaces.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMVectorMassKernel::MFEMVectorMassKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMVectorMassKernel::createMBFIntegrator()
{
  return new mfem::VectorMassIntegrator(_coef);
}

#endif
