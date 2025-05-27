//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// local includes
#include "InternalSideIndicatorBase.h"
#include "NeighborMooseVariableInterface.h"

// forward declarations
template <typename ComputeValueType>
class InternalSideIndicatorTempl;

typedef InternalSideIndicatorTempl<Real> InternalSideIndicator;
typedef InternalSideIndicatorTempl<RealVectorValue> VectorInternalSideIndicator;

/**
 * The InternalSideIndicator class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 *
 */
template <typename ComputeValueType>
class InternalSideIndicatorTempl : public InternalSideIndicatorBase,
                                   public NeighborMooseVariableInterface<ComputeValueType>
{
public:
  /**
   * Factory constructor initializes all internal references needed for indicator computation.
   *
   */
  static InputParameters validParams();

  InternalSideIndicatorTempl(const InputParameters & parameters);

protected:
  virtual bool isVarFV() const override { return _var.isFV(); }

  MooseVariableField<ComputeValueType> & _var;

  /// Holds the current solution at the current quadrature point on the face.
  const typename OutputTools<ComputeValueType>::VariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  const typename OutputTools<ComputeValueType>::VariableGradient & _grad_u;

  /// Holds the current solution at the current quadrature point
  const typename OutputTools<ComputeValueType>::VariableValue & _u_neighbor;

  /// Holds the current solution gradient at the current quadrature point
  const typename OutputTools<ComputeValueType>::VariableGradient & _grad_u_neighbor;
};

// Prevent implicit instantiation in other translation units where these classes are used
extern template class InternalSideIndicatorTempl<Real>;
extern template class InternalSideIndicatorTempl<RealVectorValue>;
