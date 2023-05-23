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
#include "NaKFluidProperties.h"

class NaKFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  NaKFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("NaKFluidProperties");
    uo_pars.set<Real>("weight_fraction_K") = 0.778;
    _fe_problem->addUserObject("NaKFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<NaKFluidProperties>("fp");
  }

  const NaKFluidProperties * _fp;
};
