//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideIndicator.h"

// forward declarations
template <typename ComputeValueType>
class ValueJumpIndicatorTempl;

typedef ValueJumpIndicatorTempl<Real> ValueJumpIndicator;
typedef ValueJumpIndicatorTempl<RealVectorValue> VectorValueJumpIndicator;

template <typename ComputeValueType>
class ValueJumpIndicatorTempl : public InternalSideIndicatorTempl<ComputeValueType>
{
public:
  static InputParameters validParams();

  ValueJumpIndicatorTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};

template <>
Real ValueJumpIndicatorTempl<Real>::computeQpIntegral();

template <>
Real ValueJumpIndicatorTempl<RealVectorValue>::computeQpIntegral();

// Prevent implicit instantiation in other translation units where these classes are used
extern template class ValueJumpIndicatorTempl<Real>;
extern template class ValueJumpIndicatorTempl<RealVectorValue>;
