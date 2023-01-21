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
#include "LeadFluidProperties.h"

class LeadFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  LeadFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("LeadFluidProperties");
    _fe_problem->addUserObject("LeadFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<LeadFluidProperties>("fp");
  }

  const LeadFluidProperties * _fp;
};
