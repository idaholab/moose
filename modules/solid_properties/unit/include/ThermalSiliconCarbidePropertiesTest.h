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
#include "ThermalSiliconCarbideProperties.h"

class ThermalSiliconCarbidePropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalSiliconCarbidePropertiesTest() : MooseObjectUnitTest("SolidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars1 = _factory.getValidParams("ThermalSiliconCarbideProperties");
    _fe_problem->addUserObject("ThermalSiliconCarbideProperties", "sp1", uo_pars1);
    _sp1 = &_fe_problem->getUserObject<ThermalSiliconCarbideProperties>("sp1");

    InputParameters uo_pars2 = _factory.getValidParams("ThermalSiliconCarbideProperties");
    uo_pars2.set<MooseEnum>("thermal_conductivity_model") = "PARFUME";
    uo_pars2.set<Real>("density") = 3000.0;
    _fe_problem->addUserObject("ThermalSiliconCarbideProperties", "sp2", uo_pars2);
    _sp2 = &_fe_problem->getUserObject<ThermalSiliconCarbideProperties>("sp2");
  }

  // model using the Snead conductivity correlation (the default)
  const ThermalSiliconCarbideProperties * _sp1;

  // model using the PARFUME conductivity correlation and non-default density
  const ThermalSiliconCarbideProperties * _sp2;
};
