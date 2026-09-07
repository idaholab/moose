//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedGradGradKernel.h"

registerMooseObject("MooseApp", MFEMMixedGradGradKernel);

InputParameters
MFEMMixedGradGradKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k\\vec\\nabla u, \\vec\\nabla v)_\\Omega$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient for property k.");
  params.addParam<MFEMMatrixCoefficientName>("matrix_coefficient",
                                             "Name of matrix coefficient for property k. Mutually "
                                             "exclusive with parameter 'coefficient'.");
  return params;
}

MFEMMixedGradGradKernel::MFEMMixedGradGradKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters),
    _variant_coef(getVariantCoefficient("coefficient", "vector_coefficient", "matrix_coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedGradGradKernel::createMBFIntegrator()
{
  return std::visit([](auto & c) { return new mfem::MixedGradGradIntegrator(c); }, _variant_coef);
}

#endif
