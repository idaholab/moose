//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TestADScalarKernel.h"

/**
 * AD scalar kernel for testing that Jacobians are correctly computed using AD
 */
class RenameCoupledScalarVarScalarKernel : public TestADScalarKernel
{
public:
  static InputParameters validParams();

  RenameCoupledScalarVarScalarKernel(const InputParameters & parameters);
};
