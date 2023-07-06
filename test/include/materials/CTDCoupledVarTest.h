//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "CompileTimeDerivativesMaterial.h"

/**
 * Test Material class that couples 3 variables, is not AD, and provides up to 3rd order
 * derivatives.
 */
class CTDCoupledVarTest : public CompileTimeDerivativesMaterial<3, false, 3>
{
public:
  static InputParameters validParams();

  CTDCoupledVarTest(const InputParameters & parameters);

protected:
  void computeQpProperties();
};
