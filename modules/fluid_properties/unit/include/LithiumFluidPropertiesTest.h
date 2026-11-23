//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"
#include "LithiumFluidProperties.h"

class LithiumFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  LithiumFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("LithiumFluidProperties");
    _fe_problem->addUserObject("LithiumFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<LithiumFluidProperties>("fp");
  }

  const LithiumFluidProperties * _fp;
};
