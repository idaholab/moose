//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STAINLESSSTEEL316TEST_H
#define STAINLESSSTEEL316TEST_H

#include "MooseObjectUnitTest.h"
#include "ThermalStainlessSteel316Properties.h"

class ThermalStainlessSteel316PropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalStainlessSteel316PropertiesTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    registerObjects(_factory);
    buildObjects();
  }

protected:
  void registerObjects(Factory & factory)
  {
    registerUserObject(ThermalStainlessSteel316Properties);
  }

  void buildObjects()
  {
    // all default parameters
    InputParameters uo_pars = _factory.getValidParams("ThermalStainlessSteel316Properties");
    _fe_problem->addUserObject("ThermalStainlessSteel316Properties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ThermalStainlessSteel316Properties>("sp");

    // oxidized surface for computing emissivity
    InputParameters uo_pars2 = _factory.getValidParams("ThermalStainlessSteel316Properties");
    uo_pars2.set<MooseEnum>("surface") = "polished";
    _fe_problem->addUserObject("ThermalStainlessSteel316Properties", "sp2", uo_pars2);
    _sp2 = &_fe_problem->getUserObject<ThermalStainlessSteel316Properties>("sp2");

    // user-specified constant emissivity
    InputParameters uo_pars3 = _factory.getValidParams("ThermalStainlessSteel316Properties");
    uo_pars3.set<Real>("emissivity") = 0.8;
    _fe_problem->addUserObject("ThermalStainlessSteel316Properties", "sp3", uo_pars3);
    _sp3 = &_fe_problem->getUserObject<ThermalStainlessSteel316Properties>("sp3");
  }

  const ThermalStainlessSteel316Properties * _sp;
  const ThermalStainlessSteel316Properties * _sp2;
  const ThermalStainlessSteel316Properties * _sp3;
};

#endif /* STAINLESSSTEEL316TEST_H */
