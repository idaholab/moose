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
#include "ThermalMonolithicSiCProperties.h"

class ThermalMonolithicSiCPropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalMonolithicSiCPropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars1 = _factory.getValidParams("ThermalMonolithicSiCProperties");
    uo_pars1.set<MooseEnum>("thermal_conductivity_model") = "SNEAD";
    _fe_problem->addUserObject("ThermalMonolithicSiCProperties", "sp1", uo_pars1);
    _sp1 = &_fe_problem->getUserObject<ThermalMonolithicSiCProperties>("sp1");

    InputParameters uo_pars2 = _factory.getValidParams("ThermalMonolithicSiCProperties");
    uo_pars2.set<MooseEnum>("thermal_conductivity_model") = "STONE";
    uo_pars2.set<Real>("density") = 3000.0;
    _fe_problem->addUserObject("ThermalMonolithicSiCProperties", "sp2", uo_pars2);
    _sp2 = &_fe_problem->getUserObject<ThermalMonolithicSiCProperties>("sp2");
  }

  // model using the Snead conductivity correlation
  const ThermalMonolithicSiCProperties * _sp1;

  // model using the Stone conductivity correlation and non-default density
  const ThermalMonolithicSiCProperties * _sp2;
};
