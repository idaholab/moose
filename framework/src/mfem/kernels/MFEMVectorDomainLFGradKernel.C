//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorDomainLFGradKernel.h"
#include "MFEMFlattenedMatrixCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorDomainLFGradKernel);

InputParameters
MFEMVectorDomainLFGradKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the linear form "
      "$(F, \\vec \\nabla \\vec v)_\\Omega$, where $\\vec v$ is an H1 vector test function and "
      "$F$ is a matrix coefficient. If only a scalar coefficient $f$ is provided, $F = f I$, "
      "giving the linear form $(f, \\vec \\nabla \\cdot \\vec v)_\\Omega$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient",
      "1.",
      "Name of the scalar coefficient f defining the isotropic tensor F = f I. Unused if "
      "'matrix_coefficient' is set.");
  params.addParam<MFEMMatrixCoefficientName>(
      "matrix_coefficient",
      "Name of the matrix coefficient F, whose entry (i, j) multiplies the derivative of "
      "component i of the test function in direction j.");
  return params;
}

MFEMVectorDomainLFGradKernel::MFEMVectorDomainLFGradKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _flattened_tensor_coef(
        getMFEMProblem().getCoefficients().declareVector<MFEMFlattenedMatrixCoefficient>(
            "__VectorDomainLFGradKernel_" +
                parameters.get<std::string>(MooseBase::unique_name_param),
            getTensorCoefficient()))
{
}

mfem::MatrixCoefficient &
MFEMVectorDomainLFGradKernel::getTensorCoefficient()
{
  if (isParamValid("matrix_coefficient"))
  {
    if (isParamSetByUser("coefficient"))
      paramError("coefficient",
                 "Only one of 'coefficient' and 'matrix_coefficient' may be set by the user.");
    return getMatrixCoefficient("matrix_coefficient");
  }

  const int dim = getMFEMProblem().mesh().getMFEMParMesh().SpaceDimension();
  mfem::DenseMatrix identity(dim);
  identity = 0.0;
  for (const auto i : make_range(dim))
    identity(i, i) = 1.0;

  auto & coefficients = getMFEMProblem().getCoefficients();
  const auto prefix =
      "__VectorDomainLFGradKernel_" + parameters().get<std::string>(MooseBase::unique_name_param);
  return coefficients.declareMatrix<mfem::ScalarMatrixProductCoefficient>(
      prefix + "_isotropic",
      getScalarCoefficient("coefficient"),
      coefficients.declareMatrix<mfem::MatrixConstantCoefficient>(prefix + "_identity", identity));
}

mfem::LinearFormIntegrator *
MFEMVectorDomainLFGradKernel::createLFIntegrator()
{
  return new mfem::VectorDomainLFGradIntegrator(_flattened_tensor_coef);
}

#endif
