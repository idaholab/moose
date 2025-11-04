//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayReactionNodalKernel.h"

#include "Function.h"

registerMooseObject("MooseApp", ArrayReactionNodalKernel);

InputParameters
ArrayReactionNodalKernel::validParams()
{
  InputParameters params = ArrayNodalKernel::validParams();
  params.addClassDescription(
      "Implements a simple consuming reaction term at nodes for an array variable");
  params.addRequiredParam<RealEigenVector>("coeff",
                                           "Coefficients for multiplying the reaction term");
  return params;
}

ArrayReactionNodalKernel::ArrayReactionNodalKernel(const InputParameters & parameters)
  : ArrayNodalKernel(parameters), _coeff(getParam<RealEigenVector>("coeff"))
{
  if (_coeff.size() != _count)
    paramError("coeff",
               "The size of the coefficient vector must match the size of the array variable");
}

void
ArrayReactionNodalKernel::computeQpResidual(RealEigenVector & residual)
{
  residual = _coeff.cwiseProduct(_u[_qp]);
}

RealEigenVector
ArrayReactionNodalKernel::computeQpJacobian()
{
  return _coeff;
}
