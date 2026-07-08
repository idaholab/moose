//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarQuadratureFunction.h"
#include "MFEMQuadratureFunctionCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMScalarQuadratureFunction);

InputParameters
MFEMScalarQuadratureFunction::validParams()
{
  InputParameters params = MFEMQuadratureFunctionBase::validParams();
  params.addClassDescription(
      "Declares a scalar MFEM coefficient holding precomputed values of a source coefficient at "
      "quadrature points. Values are (re)projected lazily when the coefficient is used.");
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "coefficient", "Scalar coefficient to project onto the quadrature points.");
  return params;
}

MFEMScalarQuadratureFunction::MFEMScalarQuadratureFunction(const InputParameters & parameters)
  : MFEMQuadratureFunctionBase(parameters), _qf(&_qspace)
{
  // Zero-initialize the storage; real values are projected lazily on first use.
  _qf = 0.0;
  getMFEMProblem().getCoefficients().declareScalar<MFEMQuadratureFunctionCoefficient>(
      name(), getScalarCoefficient("coefficient"), _qf, updatePolicy());
}

#endif
