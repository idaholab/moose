//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMGroupConvectionKernel.h"

registerMooseObject("MooseApp", MFEMGroupConvectionKernel);

InputParameters
MFEMGroupConvectionKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(\\vec Q \\cdot \\vec\\nabla u, v)_\\Omega$ arising from the weak "
                             "form of the operator $\\vec Q \\cdot \\vec\\nabla u$.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Name of the vector coefficient to use.");
  return params;
}

MFEMGroupConvectionKernel::MFEMGroupConvectionKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMGroupConvectionKernel::createBFIntegrator()
{
  return new mfem::GroupConvectionIntegrator(_vec_coef);
}

#endif
