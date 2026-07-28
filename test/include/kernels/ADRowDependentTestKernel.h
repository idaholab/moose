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

protected:
  ADReal computeQpResidual() override;

  const MooseArray<ADReal> & _ad_dof_values;
};
