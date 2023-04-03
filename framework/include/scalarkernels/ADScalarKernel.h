//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernelBase.h"

/**
 * Base class for AD scalar kernels
 */
class ADScalarKernel : public ScalarKernelBase
{
public:
  static InputParameters validParams();

  ADScalarKernel(const InputParameters & parameters);

  virtual void reinit() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  virtual ADReal computeQpResidual() = 0;

  /// The current solution
  const ADVariableValue & _u;

  /// Residuals for each order
  std::vector<ADReal> _residuals;

private:
  /**
   * Computes the Jacobian using automatic differentiation
   */
  void computeADJacobian();

  /// Flag indicating that the Jacobian has already been computed
  bool _jacobian_already_computed;
};
