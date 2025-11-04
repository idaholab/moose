//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiracKernelBase.h"
#include "MooseVariableInterface.h"
#include "ADFunctorInterface.h"

/**
 * AD version of DiracKernel
 */
class ADDiracKernel : public DiracKernelBase,
                      public MooseVariableInterface<Real>,
                      public ADFunctorInterface
{
public:
  static InputParameters validParams();

  ADDiracKernel(const InputParameters & parameters);

  virtual void jacobianSetup() override;

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;

  virtual const MooseVariableField<Real> & variable() const override { return _var; }

protected:
  /**
   * Computes the residual contribution at the current quadrature point
   */
  virtual ADReal computeQpResidual() = 0;

  /// Variable this kernel acts on
  MooseVariableField<Real> & _var;

  /// Values of shape functions at QPs
  const ADTemplateVariablePhiValue<Real> & _phi;

  /// Values of test functions at QPs
  const ADTemplateVariableTestValue<Real> & _test;

  /// Holds the solution at current quadrature points
  const ADTemplateVariableValue<Real> & _u;
  /// Holds the solution gradient at the current quadrature points
  const ADTemplateVariableGradient<Real> & _grad_u;

private:
  /// Computes the AD residuals for the current element
  void computeADResiduals();
  /// Computes the full Jacobian for the current element
  void computeFullJacobian();

  /// AD residuals for the current element
  std::vector<ADReal> _ad_residuals;
  /// The element corresponding to previous Jacobian calculation
  const Elem * _last_jacobian_elem;
};
