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
#include "StiffenedGasFluidProperties.h"

class StiffenedGasFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  StiffenedGasFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters eos_pars = _factory.getValidParams("StiffenedGasFluidProperties");
    eos_pars.set<Real>("gamma") = 2.35;
    eos_pars.set<Real>("q") = -1167e3;
    eos_pars.set<Real>("q_prime") = 0;
    eos_pars.set<Real>("p_inf") = 1.e9;
    eos_pars.set<Real>("cv") = 1816;
    eos_pars.set<std::string>("_object_name") = "name3";
    _fe_problem->addUserObject("StiffenedGasFluidProperties", "fp", eos_pars);
    _fp = &_fe_problem->getUserObject<StiffenedGasFluidProperties>("fp");
  }

  const StiffenedGasFluidProperties * _fp;
};
