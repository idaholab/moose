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
#include "DualRealOps.h"

// Forward declarations
template <typename>
class ADRayKernelTempl;

using ADRayKernel = ADRayKernelTempl<Real>;
// Not implementing this until there is a use case and tests for it!
// using ADVectorRayKernel = ADRayKernelTempl<RealVectorValue>;

/**
 * Base class for an AD ray kernel that contributes to the residual and/or Jacobian
 */
template <typename T>
class ADRayKernelTempl : public IntegralRayKernelBase,
                         public MooseVariableInterface<T>,
                         public TaggingInterface
{
public:
  ADRayKernelTempl(const InputParameters & params);

  static InputParameters validParams();

  /**
   * The MooseVariable this RayKernel contributes to
   */
  MooseVariableField<T> & variable() { return _var; }

  void onSegment() override final;

protected:
  /**
   * Compute this kernel's contribution to the residual at _qp and _i
   */
  virtual ADReal computeQpResidual() = 0;

  /**
   * Insertion point for calculation before the residual computation
   */
  virtual void precalculateResidual(){};

  /// The MooseVariable this kernel contributes to
  MooseVariableField<T> & _var;

  /// The current test function
  const ADTemplateVariableTestValue<T> & _test;

  /// Holds the solution at current quadrature points
  const ADTemplateVariableValue<T> & _u;

  /// Holds the solution gradient at the current quadrature points
  const ADTemplateVariableGradient<T> & _grad_u;

  /// The current shape functions
  const ADTemplateVariablePhiValue<T> & _phi;

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

  /// Temporary for filling the residuals
  std::vector<ADReal> _residuals;
};
