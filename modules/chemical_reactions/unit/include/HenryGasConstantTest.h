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
#include "HenryGasConstant.h"

class HenryGasConstantTest : public MooseObjectUnitTest
{
public:
  HenryGasConstantTest() : MooseObjectUnitTest("ChemicalReactionsApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    // Set up object for helium and FLiBe
    {
      InputParameters params = _factory.getValidParams("HenryGasConstant");
      params.set<MooseEnum>("salt") = "FLIBE";
      params.set<Real>("radius") = 1.4e-10;
      _fe_problem->addUserObject("HenryGasConstant", "henry_helium_flibe", params);
      _henry_helium_flibe = &_fe_problem->getUserObject<HenryGasConstant>("henry_helium_flibe");
    }
    // Set up object for argon and FLiBe
    {
      InputParameters params = _factory.getValidParams("HenryGasConstant");
      params.set<MooseEnum>("salt") = "FLIBE";
      params.set<Real>("radius") = 1.88e-10;
      _fe_problem->addUserObject("HenryGasConstant", "henry_argon_flibe", params);
      _henry_argon_flibe = &_fe_problem->getUserObject<HenryGasConstant>("henry_argon_flibe");
    }
    // Set up object for helium and FLiNaK
    {
      InputParameters params = _factory.getValidParams("HenryGasConstant");
      params.set<MooseEnum>("salt") = "FLINAK";
      params.set<Real>("radius") = 1.4e-10;
      _fe_problem->addUserObject("HenryGasConstant", "henry_helium_flinak", params);
      _henry_helium_flinak = &_fe_problem->getUserObject<HenryGasConstant>("henry_helium_flinak");
    }
    // Set up object for argon and FLiNaK
    {
      InputParameters params = _factory.getValidParams("HenryGasConstant");
      params.set<MooseEnum>("salt") = "FLINAK";
      params.set<Real>("radius") = 1.88e-10;
      _fe_problem->addUserObject("HenryGasConstant", "henry_argon_flinak", params);
      _henry_argon_flinak = &_fe_problem->getUserObject<HenryGasConstant>("henry_argon_flinak");
    }
  }

  const HenryGasConstant * _henry_argon_flibe;
  const HenryGasConstant * _henry_helium_flibe;
  const HenryGasConstant * _henry_argon_flinak;
  const HenryGasConstant * _henry_helium_flinak;
};
