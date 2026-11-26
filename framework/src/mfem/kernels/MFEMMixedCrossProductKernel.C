//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedCrossProductKernel.h"

registerMooseObject("MooseApp", MFEMMixedCrossProductKernel);

InputParameters
MFEMMixedCrossProductKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the mixed bilinear "
                             "form $(\\vec V \\times \\vec u, \\vec v)_\\Omega$.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Name of the vector coefficient to use.");
  return params;
}

MFEMMixedCrossProductKernel::MFEMMixedCrossProductKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedCrossProductKernel::createMBFIntegrator()
{
  return new mfem::MixedCrossProductIntegrator(_vec_coef);
}

#endif
