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
#include "MechanicalCopperProperties.h"

class MechanicalCopperPropertiesTest : public MooseObjectUnitTest
{
public:
  MechanicalCopperPropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("MechanicalCopperProperties");
    _fe_problem->addUserObject("MechanicalCopperProperties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<MechanicalCopperProperties>("sp");
  }

  const MechanicalCopperProperties * _sp;
};
