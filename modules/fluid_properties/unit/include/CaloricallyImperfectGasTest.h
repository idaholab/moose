//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"
#include "CaloricallyImperfectGas.h"
#include "IdealGasFluidProperties.h"

class CaloricallyImperfectGasTest : public MooseObjectUnitTest
{
public:
  CaloricallyImperfectGasTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters default_fn_params = _factory.getValidParams("ConstantFunction");
    default_fn_params.set<Real>("value") = 1.0;
    _fe_problem->addFunction("ConstantFunction", "default_fn", default_fn_params);
  }
};
