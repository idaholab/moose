//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ValueJumpIndicator.h"

registerMooseObject("MooseApp", ValueJumpIndicator);
registerMooseObject("MooseApp", VectorValueJumpIndicator);

template <typename ComputeValueType>
InputParameters
ValueJumpIndicatorTempl<ComputeValueType>::validParams()
{
  InputParameters params = InternalSideIndicatorTempl<ComputeValueType>::validParams();
  params.addClassDescription("Compute the jump of the solution across element bondaries.");
  return params;
}

template <typename ComputeValueType>
ValueJumpIndicatorTempl<ComputeValueType>::ValueJumpIndicatorTempl(
    const InputParameters & parameters)
  : InternalSideIndicatorTempl<ComputeValueType>(parameters)
{
}

template <>
Real
ValueJumpIndicatorTempl<Real>::computeQpIntegral()
{
  Real jump = _u[_qp] - _u_neighbor[_qp];

  return jump * jump;
}

template <>
Real
ValueJumpIndicatorTempl<RealVectorValue>::computeQpIntegral()
{
  Real jump = _normals[_qp] * (_u[_qp] - _u_neighbor[_qp]);

  return jump * jump;
}

// Explicitly instantiate two versions of the ValueJumpIndicatorTempl class
template class ValueJumpIndicatorTempl<Real>;
template class ValueJumpIndicatorTempl<RealVectorValue>;
