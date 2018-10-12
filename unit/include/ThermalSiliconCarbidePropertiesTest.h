//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIC_TEST_H
#define SIC_TEST_H

#include "MooseObjectUnitTest.h"
#include "ThermalSiliconCarbideProperties.h"

class ThermalSiliconCarbidePropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalSiliconCarbidePropertiesTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    registerObjects(_factory);
    buildObjects();
  }

protected:
  void registerObjects(Factory & factory) { registerUserObject(ThermalSiliconCarbideProperties); }

  void buildObjects()
  {
    // all default parameters
    InputParameters uo_pars = _factory.getValidParams("ThermalSiliconCarbideProperties");
    _fe_problem->addUserObject("ThermalSiliconCarbideProperties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ThermalSiliconCarbideProperties>("sp");

    // user-specified density and alternative thermal conductivity model
    InputParameters uo_pars2 = _factory.getValidParams("ThermalSiliconCarbideProperties");
    uo_pars2.set<MooseEnum>("thermal_conductivity_model") = "parfume";
    uo_pars2.set<Real>("density") = 3180.0;
    _fe_problem->addUserObject("ThermalSiliconCarbideProperties", "sp2", uo_pars2);
    _sp2 = &_fe_problem->getUserObject<ThermalSiliconCarbideProperties>("sp2");
  }

  const ThermalSiliconCarbideProperties * _sp;
  const ThermalSiliconCarbideProperties * _sp2;
};

#endif /* SIC_TEST_H */
