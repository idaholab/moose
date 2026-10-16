//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedVectorGradientKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMixedVectorGradientKernel);

InputParameters
MFEMMixedVectorGradientKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k\\vec\\nabla u, \\vec v)_\\Omega$ "
      "arising from the weak form of the gradient operator "
      "$k\\vec \\nabla u$.");
  params.addParam<MFEMScalarCoefficientName>(
      MFEMKernel::COEFFICIENT_PARAM,
      "1.",
      "Name of scalar coefficient k to multiply the integrator by.");
  params.addParam<MFEMMatrixCoefficientName>(MFEMKernel::MATRIX_COEFFICIENT_PARAM,
                                             "Name of matrix coefficient for property k. Mutually "
                                             "exclusive with parameter 'coefficient'.");
  return params;
}

MFEMMixedVectorGradientKernel::MFEMMixedVectorGradientKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters)
{
}

mfem::BilinearFormIntegrator *
MFEMMixedVectorGradientKernel::createMBFIntegrator()
{
  auto coeffs = getMFEMProblem().getCoefficients().resolveCoefficientVariant(
      _pars, MFEMKernel::COEFFICIENT_PARAM, MFEMKernel::MATRIX_COEFFICIENT_PARAM);
  return std::visit([](auto & c) { return new mfem::MixedVectorGradientIntegrator(c); }, coeffs);
}

#endif
