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
#include "MFEMPMLMatrixCoefficient.h"
#include "MFEMProblem.h"

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
      "coefficient", "1.", "Name of scalar coefficient k to multiply the integrator by.");
  params.addParam<MFEMMatrixCoefficientName>(
      "matrix_coefficient",
      "Name of matrix coefficient k to multiply the integrator by, in place of the scalar "
      "'coefficient'. Only available in three dimensions, where the curl of a vector field is "
      "itself a vector.");
  return params;
}

MFEMCurlCurlKernel::MFEMCurlCurlKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient("coefficient")),
    _matrix_coef(isParamValid("matrix_coefficient") ? &getMatrixCoefficient("matrix_coefficient")
                                                    : nullptr)
// FIXME: The MFEM bilinear form can also handle vector coefficients, so ideally we'd handle those
// too.
{
  if (_matrix_coef && isParamSetByUser("coefficient"))
    mooseError("Only one of 'coefficient' and 'matrix_coefficient' may be set.");

  // A perfectly matched layer stretches the curl of the field by a different tensor from the field
  // itself, and this integrator integrates the curl.
  if (const auto * const pml = dynamic_cast<const MFEMPMLMatrixCoefficient *>(_matrix_coef);
      pml && pml->tensorType() != MFEMPMLMatrixCoefficient::CURL)
    mooseError("The perfectly matched layer coefficient '",
               getParam<MFEMMatrixCoefficientName>("matrix_coefficient"),
               "' stretches the field rather than its curl. Declare it with 'tensor = curl'.");
}

mfem::BilinearFormIntegrator *
MFEMCurlCurlKernel::createBFIntegrator()
{
  if (_matrix_coef)
    return new mfem::CurlCurlIntegrator(*_matrix_coef);

  return new mfem::CurlCurlIntegrator(_coef);
}

#endif
