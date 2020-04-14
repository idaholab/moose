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
#include "PorousFlowCapillaryPressureVG.h"
#include "PorousFlowWaterNCG.h"
#include "Water97FluidProperties.h"
#include "CO2FluidProperties.h"

class PorousFlowFluidStateFlashTest : public MooseObjectUnitTest
{
public:
  PorousFlowFluidStateFlashTest() : MooseObjectUnitTest("PorousFlowApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters pc_params = _factory.getValidParams("PorousFlowCapillaryPressureVG");
    pc_params.set<Real>("m") = 0.5;
    pc_params.set<Real>("alpha") = 0.1;
    _fe_problem->addUserObject("PorousFlowCapillaryPressureVG", "pc", pc_params);
    _pc = &_fe_problem->getUserObject<PorousFlowCapillaryPressureVG>("pc");

    InputParameters water_params = _factory.getValidParams("Water97FluidProperties");
    _fe_problem->addUserObject("Water97FluidProperties", "water_fp", water_params);
    _water_fp = &_fe_problem->getUserObject<Water97FluidProperties>("water_fp");

    InputParameters ncg_params = _factory.getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "ncg_fp", ncg_params);
    _ncg_fp = &_fe_problem->getUserObject<CO2FluidProperties>("ncg_fp");

    InputParameters uo_params = _factory.getValidParams("PorousFlowWaterNCG");
    uo_params.set<UserObjectName>("water_fp") = "water_fp";
    uo_params.set<UserObjectName>("gas_fp") = "ncg_fp";
    uo_params.set<UserObjectName>("capillary_pressure") = "pc";
    _fe_problem->addUserObject("PorousFlowWaterNCG", "fp", uo_params);
    _fp = &_fe_problem->getUserObject<PorousFlowWaterNCG>("fp");
  }

  const PorousFlowCapillaryPressureVG * _pc;
  const PorousFlowWaterNCG * _fp;
  const Water97FluidProperties * _water_fp;
  const CO2FluidProperties * _ncg_fp;
};
