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
#include "ThermalSS316Properties.h"

class ThermalSS316PropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalSS316PropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("ThermalSS316Properties");
    _fe_problem->addUserObject("ThermalSS316Properties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ThermalSS316Properties>("sp");
  }

  const ThermalSS316Properties * _sp;
};
