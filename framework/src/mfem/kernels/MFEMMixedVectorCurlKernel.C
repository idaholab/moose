//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedVectorCurlKernel.h"

registerMooseObject("MooseApp", MFEMMixedVectorCurlKernel);

InputParameters
MFEMMixedVectorCurlKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form $(k \\vec\\nabla "
      "\\times \\vec u, \\vec v)_\\Omega$ arising from the weak form of the operator $k "
      "\\vec\\nabla \\times \\vec u$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient",
      "1.",
      "Name of scalar property k to use. Cannot be set together with vector_coefficient.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "Name of the vector coefficient to use in place of k, applied as a diagonal matrix.");
  return params;
}

MFEMMixedVectorCurlKernel::MFEMMixedVectorCurlKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters),
    _coef(getScalarCoefficient("coefficient")),
    _vec_coef(isParamValid("vector_coefficient") ? &getVectorCoefficient("vector_coefficient")
                                                 : nullptr)
// FIXME: The MFEM bilinear form can also handle matrix coefficients.
{
  if (_vec_coef && isParamSetByUser("coefficient"))
    paramError("vector_coefficient", "Cannot be set together with 'coefficient'.");
}

mfem::BilinearFormIntegrator *
MFEMMixedVectorCurlKernel::createMBFIntegrator()
{
  return _vec_coef ? new mfem::MixedVectorCurlIntegrator(*_vec_coef)
                   : new mfem::MixedVectorCurlIntegrator(_coef);
}

#endif
