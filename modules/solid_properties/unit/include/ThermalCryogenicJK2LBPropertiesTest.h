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
#include "ThermalCryogenicJK2LBProperties.h"

class ThermalCryogenicJK2LBPropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalCryogenicJK2LBPropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("ThermalCryogenicJK2LBProperties");
    _fe_problem->addUserObject("ThermalCryogenicJK2LBProperties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ThermalCryogenicJK2LBProperties>("sp");
  }

  const ThermalCryogenicJK2LBProperties * _sp;
};
