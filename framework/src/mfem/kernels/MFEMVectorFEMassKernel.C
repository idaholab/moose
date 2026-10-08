//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEMassKernel.h"
#include "MFEMPMLMatrixCoefficient.h"
#include "MFEMPMLScalarCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFEMassKernel);

InputParameters
MFEMVectorFEMassKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k \\vec u, \\vec v)_\\Omega$ "
                             "arising from the weak form of the mass operator "
                             "$k \\vec u$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property k to multiply the integrator by");
  params.addParam<MFEMMatrixCoefficientName>(
      "matrix_coefficient",
      "Name of matrix property k to multiply the integrator by, in place of the scalar "
      "'coefficient'.");
  return params;
}

MFEMVectorFEMassKernel::MFEMVectorFEMassKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient("coefficient")),
    _matrix_coef(isParamValid("matrix_coefficient") ? &getMatrixCoefficient("matrix_coefficient")
                                                    : nullptr)
// FIXME: The MFEM bilinear form can also handle vector coefficients, so ideally we'd handle those
// too.
{
  if (_matrix_coef && isParamSetByUser("coefficient"))
    mooseError("Only one of 'coefficient' and 'matrix_coefficient' may be set.");

  // The field is a vector in any dimension, so the tensor stretching it never reduces to a scalar
  // the way the one stretching its curl does in two dimensions.
  if (dynamic_cast<const MFEMPMLScalarCoefficient *>(&_coef))
    mooseError("The perfectly matched layer coefficient '",
               getParam<MFEMScalarCoefficientName>("coefficient"),
               "' is the scalar the stretched curl curl coefficient reduces to in two dimensions. "
               "Pass a matrix coefficient declared with 'tensor = field' as 'matrix_coefficient' "
               "instead.");

  // A perfectly matched layer stretches the field by a different tensor from its curl, and this
  // integrator integrates the field.
  if (const auto * const pml = dynamic_cast<const MFEMPMLMatrixCoefficient *>(_matrix_coef);
      pml && pml->tensorType() != MFEMPMLMatrixCoefficient::FIELD)
    mooseError("The perfectly matched layer coefficient '",
               getParam<MFEMMatrixCoefficientName>("matrix_coefficient"),
               "' stretches the curl of the field rather than the field. Declare it with 'tensor = "
               "field'.");
}

mfem::BilinearFormIntegrator *
MFEMVectorFEMassKernel::createBFIntegrator()
{
  if (_matrix_coef)
    return new mfem::VectorFEMassIntegrator(*_matrix_coef);

  return new mfem::VectorFEMassIntegrator(_coef);
}

#endif
