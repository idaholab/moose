//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorQuadratureFunction.h"
#include "MFEMVectorQuadratureFunctionCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorQuadratureFunction);

InputParameters
MFEMVectorQuadratureFunction::validParams()
{
  InputParameters params = MFEMQuadratureFunctionBase::validParams();
  params.addClassDescription(
      "Declares a vector MFEM coefficient holding precomputed values of a source vector "
      "coefficient at quadrature points. Values are (re)projected lazily when the coefficient "
      "is used.");
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "Vector coefficient to project onto the quadrature points.");
  return params;
}

MFEMVectorQuadratureFunction::MFEMVectorQuadratureFunction(const InputParameters & parameters)
  : MFEMQuadratureFunctionBase(parameters), _qf(&_qspace)
{
  mfem::VectorCoefficient & source = getVectorCoefficient("vector_coefficient");
  _qf.SetVDim(source.GetVDim());
  _qf = 0.0;
  getMFEMProblem().getCoefficients().declareVector<MFEMVectorQuadratureFunctionCoefficient>(
      name(), source, _qf, updatePolicy());
}

#endif
