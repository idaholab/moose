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
#include "GasLiquidMassTransfer.h"
#include "IdealGasFluidProperties.h"

class GasLiquidMassTransferTest : public MooseObjectUnitTest
{
public:
  GasLiquidMassTransferTest()
    : MooseObjectUnitTest("ScalarTransportApp")
  {
    buildObjects();
  }

protected:

  void buildObjects()
  {
    {
      InputParameters eos_pars = _factory.getValidParams("IdealGasFluidProperties");
      eos_pars.set<Real>("molar_mass") = 30e-3;
      eos_pars.set<Real>("mu") = 20e-6;
      _fe_problem->addUserObject("IdealGasFluidProperties", "fp", eos_pars);
    }

     // Set up object for Stokes-Einstein
     {
        InputParameters params = _factory.getValidParams("GasLiquidMassTransfer");
        params.set<MooseEnum>("equation") = "StokesEinstein";
        params.set<Real>("d") = 10e-3;
        params.set<UserObjectName>("fp") = "fp";
        params.set<Real>("radius") = 1e-3;
        _fe_problem->addUserObject("GasLiquidMassTransfer", "mtc_stokes", params);
        _mtc_stokes = &_fe_problem->getUserObject<GasLiquidMassTransfer>("mtc_stokes");
     }

     // Set up object for Wilke-Chang
     {
        InputParameters params = _factory.getValidParams("GasLiquidMassTransfer");
        params.set<MooseEnum>("equation") = "WilkeChang";
        params.set<Real>("d") = 10e-3;
        params.set<UserObjectName>("fp") = "fp";
        params.set<Real>("molar_weight") = 30e-3;
        _fe_problem->addUserObject("GasLiquidMassTransfer", "mtc_wilkes", params);
        _mtc_wilkes = &_fe_problem->getUserObject<GasLiquidMassTransfer>("mtc_wilkes");
     }

  }

  const GasLiquidMassTransfer * _mtc_stokes;
  const GasLiquidMassTransfer * _mtc_wilkes;

};
