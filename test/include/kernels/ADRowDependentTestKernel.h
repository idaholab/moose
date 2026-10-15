//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "ADKernel.h"

/**
 * Test kernel whose residual rows have distinct AD derivative support.
 */
class ADRowDependentTestKernel : public ADKernel
{
public:
  static InputParameters validParams();

  ADRowDependentTestKernel(const InputParameters & parameters);

  void jacobianSetup() override;

protected:
  void computeJacobian() override;
  void computeResidualAndJacobian() override;
  void computeOffDiagJacobian(unsigned int) override;
  ADReal computeQpResidual() override;

  const MooseArray<ADReal> & _ad_dof_values;

  /// Last element assembled through the off-diagonal Jacobian callback
  const Elem * _last_jacobian_elem;
};
