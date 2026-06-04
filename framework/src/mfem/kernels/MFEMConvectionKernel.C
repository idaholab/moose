//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMConvectionKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMConvectionKernel);

InputParameters
MFEMConvectionKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(\\vec\\beta \\cdot \\vec\\nabla u, v)_\\Omega$ "
                             "arising from the weak form of the convection operator "
                             "$\\vec\\beta \\cdot \\vec\\nabla u$.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "1. 0. 0.",
      "The name of the velocity vector coefficient $\\vec\\beta$.");
  return params;
}

MFEMConvectionKernel::MFEMConvectionKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMConvectionKernel::createBFIntegrator()
{
  return new mfem::ConvectionIntegrator(_vec_coef);
}

#endif
