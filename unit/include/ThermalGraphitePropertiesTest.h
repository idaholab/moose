//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GRAPHITE_TEST_H
#define GRAPHITE_TEST_H

#include "MooseObjectUnitTest.h"
#include "ThermalGraphiteProperties.h"

class ThermalGraphitePropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalGraphitePropertiesTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    registerObjects(_factory);
    buildObjects();
  }

protected:
  void registerObjects(Factory & factory) { registerUserObject(ThermalGraphiteProperties); }

  void buildObjects()
  {
    // all default parameters
    InputParameters uo_pars = _factory.getValidParams("ThermalGraphiteProperties");
    _fe_problem->addUserObject("ThermalGraphiteProperties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ThermalGraphiteProperties>("sp");

    // non-default emissivity correlation
    InputParameters uo_pars2 = _factory.getValidParams("ThermalGraphiteProperties");
    uo_pars2.set<MooseEnum>("surface") = "polished";
    _fe_problem->addUserObject("ThermalGraphiteProperties", "sp2", uo_pars2);
    _sp2 = &_fe_problem->getUserObject<ThermalGraphiteProperties>("sp2");

    // user-specified emissivity and density
    InputParameters uo_pars3 = _factory.getValidParams("ThermalGraphiteProperties");
    uo_pars3.set<Real>("density_room_temp") = 1885.0;
    uo_pars3.set<Real>("emissivity") = 0.8;
    _fe_problem->addUserObject("ThermalGraphiteProperties", "sp3", uo_pars3);
    _sp3 = &_fe_problem->getUserObject<ThermalGraphiteProperties>("sp3");
  }

  const ThermalGraphiteProperties * _sp;
  const ThermalGraphiteProperties * _sp2;
  const ThermalGraphiteProperties * _sp3;
};

#endif /* GRAPHITE_TEST_H */
