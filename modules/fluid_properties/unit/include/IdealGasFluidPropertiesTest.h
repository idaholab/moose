//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  IdealGasFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp"), _molar_mass(0.028966206103678928) { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("IdealGasFluidProperties");
    uo_pars.set<Real>("molar_mass") = _molar_mass;
    uo_pars.set<Real>("gamma") = 1.41;
    uo_pars.set<Real>("e_ref") = 1000.0;
    _fe_problem->addUserObject("IdealGasFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<IdealGasFluidProperties>("fp");
  }

  const IdealGasFluidProperties * _fp;

  const Real _molar_mass;
};
