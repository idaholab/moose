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
#include "MechanicalCryogenicJK2LBProperties.h"

class MechanicalCryogenicJK2LBPropertiesTest : public MooseObjectUnitTest
{
public:
  MechanicalCryogenicJK2LBPropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("MechanicalCryogenicJK2LBProperties");
    _fe_problem->addUserObject("MechanicalCryogenicJK2LBProperties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<MechanicalCryogenicJK2LBProperties>("sp");
  }

  const MechanicalCryogenicJK2LBProperties * _sp;
};
