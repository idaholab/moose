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
#include "IdealGasFluidProperties.h"

class IdealGasFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  IdealGasFluidPropertiesTest() : MooseObjectUnitTest("MooseUnitApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("IdealGasFluidProperties");
    uo_pars.set<Real>("molar_mass") = 0.028966206103678928;
    uo_pars.set<Real>("gamma") = 1.41;
    _fe_problem->addUserObject("IdealGasFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObjectTempl<IdealGasFluidProperties>("fp");

    InputParameters uo_pars_pT = _factory.getValidParams("IdealGasFluidProperties");
    _fe_problem->addUserObject("IdealGasFluidProperties", "fp_pT", uo_pars_pT);
    _fp_pT = &_fe_problem->getUserObjectTempl<IdealGasFluidProperties>("fp_pT");
  }

  const IdealGasFluidProperties * _fp;
  const IdealGasFluidProperties * _fp_pT;
};
