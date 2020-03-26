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
#include "SimpleFluidProperties.h"

class SimpleFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  SimpleFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_params = _factory.getValidParams("SimpleFluidProperties");
    _fe_problem->addUserObject("SimpleFluidProperties", "fp", uo_params);
    _fp = &_fe_problem->getUserObject<SimpleFluidProperties>("fp");

    InputParameters uo2_params = _factory.getValidParams("SimpleFluidProperties");
    uo2_params.set<Real>("porepressure_coefficient") = 0.0;
    _fe_problem->addUserObject("SimpleFluidProperties", "fp2", uo2_params);
    _fp2 = &_fe_problem->getUserObject<SimpleFluidProperties>("fp2");
  }

  const SimpleFluidProperties * _fp;
  const SimpleFluidProperties * _fp2;
};
