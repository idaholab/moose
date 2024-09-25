//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Local Includes
#include "IntegralRayKernelBase.h"

// MOOSE Includes
#include "MooseVariableInterface.h"
#include "TaggingInterface.h"

// Forward declarations
template <typename>
class RayKernelTempl;

using RayKernel = RayKernelTempl<Real>;
// Not implementing this until there is a use case and tests for it!
// using VectorRayKernel = RayKernelTempl<RealVectorValue>;

/**
 * Base class for a ray kernel that contributes to the residual and/or Jacobian
 */
template <typename T>
class RayKernelTempl : public IntegralRayKernelBase,
                       public MooseVariableInterface<T>,
                       public TaggingInterface
{
public:
  RayKernelTempl(const InputParameters & params);

  static InputParameters validParams();

  /**
   * The MooseVariable this RayKernel contributes to
   */
  MooseVariableFE<T> & variable() { return _var; }

  void onSegment() override final;

protected:
  /**
   * Compute this RayKernel's contribution to the residual at _qp and _i
   */
  virtual Real computeQpResidual() = 0;
  /**
   * Compute this RayKernel's contribution to the residual at _qp, _i, and _j
   */
  virtual Real computeQpJacobian() { return 0; }
  /**
   * Compute this RayKernel's contribution to the off diagonal jacobian for the variable numbered
   * jvar_num.
   */
  virtual Real computeQpOffDiagJacobian(const unsigned int /* jvar_num */) { return 0; }

  /**
   * Insertion point for calculation before the residual contribution
   */
  virtual void precalculateResidual(){};
  /**
   * Insertion point for calculation before the Jacobian contribution
   */
  virtual void precalculateJacobian(){};
  /**
   * Insertion point for calculation before an off-diagonal Jacobian contribution
   */
  virtual void precalculateOffDiagJacobian(unsigned int /* jvar_num */){};

  /// This is a regular kernel so we cast to a regular MooseVariable
  MooseVariableFE<T> & _var;

  /// Holds the solution at current quadrature points
  const typename OutputTools<T>::VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const typename OutputTools<T>::VariableGradient & _grad_u;

  /// Test function
  const typename OutputTools<T>::VariableTestValue & _test;

  /// Gradient of the test function
  const typename OutputTools<T>::VariableTestGradient & _grad_test;

  /// Current shape functions
  const typename OutputTools<T>::VariablePhiValue & _phi;

  /// gradient of the shape function
  const typename OutputTools<T>::VariablePhiGradient & _grad_phi;

  /// Current index for the test function
  unsigned int _i;

  /// Current index for the shape function
  unsigned int _j;

private:
  /**
   * Computes and contributes to the Jacobian for a segment
   */
  void computeJacobian();
  /**
   * Computes and contributes to the residual for a segment
   */
  void computeResidual();
};

extern template class RayKernelTempl<Real>;
