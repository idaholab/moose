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
#include "LeadLithiumFluidProperties.h"

class LeadLithiumFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  LeadLithiumFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("LeadLithiumFluidProperties");
    _fe_problem->addUserObject("LeadLithiumFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<LeadLithiumFluidProperties>("fp");
  }

  const LeadLithiumFluidProperties * _fp;
};
