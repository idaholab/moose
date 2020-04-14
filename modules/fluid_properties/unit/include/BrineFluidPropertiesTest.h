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

#include "BrineFluidProperties.h"

class BrineFluidProperties;
class SinglePhaseFluidProperties;

class BrineFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  BrineFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("BrineFluidProperties");
    _fe_problem->addUserObject("BrineFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<BrineFluidProperties>("fp");
    _water_fp = &_fp->getComponent(BrineFluidProperties::WATER);
  }

  const BrineFluidProperties * _fp;
  const SinglePhaseFluidProperties * _water_fp;
};
