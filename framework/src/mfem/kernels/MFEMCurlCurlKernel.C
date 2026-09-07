//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCurlCurlKernel.h"

registerMooseObject("MooseApp", MFEMCurlCurlKernel);

InputParameters
MFEMCurlCurlKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form "
      "$(k\\vec\\nabla \\times \\vec u, \\vec\\nabla \\times \\vec v)_\\Omega$ "
      "arising from the weak form of the curl curl operator "
      "$k\\vec\\nabla \\times \\vec\\nabla \\times \\vec u$.");
  params.addParam<MFEMScalarCoefficientName>(
      MFEMKernel::COEFFICIENT_PARAM,
      "1.",
      "Name of scalar coefficient k to multiply the integrator by.");
  params.addParam<MFEMMatrixCoefficientName>(MFEMKernel::MATRIX_COEFFICIENT_PARAM,
                                             "Name of matrix coefficient for property k. Mutually "
                                             "exclusive with parameter 'coefficient'.");
  return params;
}

MFEMCurlCurlKernel::MFEMCurlCurlKernel(const InputParameters & parameters) : MFEMKernel(parameters)
{
}

mfem::BilinearFormIntegrator *
MFEMCurlCurlKernel::createBFIntegrator()
{
  auto coeffs = getMFEMProblem().getCoefficients().resolveCoefficientVariant(
      _pars, MFEMKernel::COEFFICIENT_PARAM, MFEMKernel::MATRIX_COEFFICIENT_PARAM);
  return std::visit([](auto & c) { return new mfem::CurlCurlIntegrator(c); }, coeffs);
}

#endif
