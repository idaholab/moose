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
#include "ThermalGraphiteProperties.h"

class ThermalGraphitePropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalGraphitePropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars1 = _factory.getValidParams("ThermalGraphiteProperties");
    uo_pars1.set<MooseEnum>("grade") = "H_451";
    _fe_problem->addUserObject("ThermalGraphiteProperties", "sp1", uo_pars1);
    _sp1 = &_fe_problem->getUserObject<ThermalGraphiteProperties>("sp1");

    InputParameters uo_pars2 = _factory.getValidParams("ThermalGraphiteProperties");
    uo_pars2.set<MooseEnum>("grade") = "H_451";
    uo_pars2.set<Real>("density") = 2000.0;
    _fe_problem->addUserObject("ThermalGraphiteProperties", "sp2", uo_pars2);
    _sp2 = &_fe_problem->getUserObject<ThermalGraphiteProperties>("sp2");
  }

  // model using the H-451 grade
  const ThermalGraphiteProperties * _sp1;

  // model using the H-451 grade and non-default density
  const ThermalGraphiteProperties * _sp2;
};
