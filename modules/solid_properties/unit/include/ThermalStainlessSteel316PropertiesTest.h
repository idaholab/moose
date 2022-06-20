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
#include "ThermalStainlessSteel316Properties.h"

class ThermalStainlessSteel316PropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalStainlessSteel316PropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("ThermalStainlessSteel316Properties");
    _fe_problem->addUserObject("ThermalStainlessSteel316Properties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ThermalStainlessSteel316Properties>("sp");
  }

  const ThermalStainlessSteel316Properties * _sp;
};
