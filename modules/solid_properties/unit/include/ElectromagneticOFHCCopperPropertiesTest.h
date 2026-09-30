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
#include "ElectromagneticOFHCCopperProperties.h"

class ElectromagneticOFHCCopperPropertiesTest : public MooseObjectUnitTest
{
public:
  ElectromagneticOFHCCopperPropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("ElectromagneticOFHCCopperProperties");
    // Use default RRR = 100
    _fe_problem->addUserObject("ElectromagneticOFHCCopperProperties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ElectromagneticOFHCCopperProperties>("sp");
  }

  const ElectromagneticOFHCCopperProperties * _sp;
};
