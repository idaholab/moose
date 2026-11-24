//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedVectorMassKernel.h"

registerMooseObject("MooseApp", MFEMMixedVectorMassKernel);

InputParameters
MFEMMixedVectorMassKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k\\vec u, \\vec v)_\\Omega$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient k to multiply the integrator by.");
  params.addParam<MFEMVectorCoefficientName>(
      "diagonal_coefficient",
      "Name of diagonal coefficient for property k. Mutually exclusive with parameters "
      "'coefficient' and 'matrix_coefficient'.");
  params.addParam<MFEMMatrixCoefficientName>(
      "matrix_coefficient",
      "Name of matrix coefficient for property k. Mutually exclusive with parameters 'coefficient' "
      "and 'diagonal_coefficient'.");
  return params;
}

MFEMMixedVectorMassKernel::MFEMMixedVectorMassKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters),
    _variant_coef(
        getVariantCoefficient("coefficient", "diagonal_coefficient", "matrix_coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedVectorMassKernel::createMBFIntegrator()
{
  return std::visit([](auto & c) { return new mfem::MixedVectorMassIntegrator(c); }, _variant_coef);
}

#endif
