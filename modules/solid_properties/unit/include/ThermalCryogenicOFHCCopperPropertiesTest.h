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
#include "ThermalCryogenicOFHCCopperProperties.h"

class ThermalCryogenicOFHCCopperPropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalCryogenicOFHCCopperPropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("ThermalCryogenicOFHCCopperProperties");
    _fe_problem->addUserObject("ThermalCryogenicOFHCCopperProperties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ThermalCryogenicOFHCCopperProperties>("sp");
  }

  const ThermalCryogenicOFHCCopperProperties * _sp;
};
