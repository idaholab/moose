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
#include "PorousFlowWaterVapor.h"
#include "Water97FluidProperties.h"

class PorousFlowWaterVaporTest : public MooseObjectUnitTest
{
public:
  PorousFlowWaterVaporTest() : MooseObjectUnitTest("PorousFlowApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters pc_params = _factory.getValidParams("PorousFlowCapillaryPressureVG");
    pc_params.set<Real>("m") = 0.5;
    pc_params.set<Real>("alpha") = 1.0e-4;
    pc_params.set<Real>("pc_max") = 1.0e5;
    _fe_problem->addUserObject("PorousFlowCapillaryPressureVG", "pc", pc_params);
    _pc = &_fe_problem->getUserObject<PorousFlowCapillaryPressureVG>("pc");

    InputParameters water_params = _factory.getValidParams("Water97FluidProperties");
    _fe_problem->addUserObject("Water97FluidProperties", "water_fp", water_params);
    _water_fp = &_fe_problem->getUserObject<Water97FluidProperties>("water_fp");

    InputParameters uo_params = _factory.getValidParams("PorousFlowWaterVapor");
    uo_params.set<UserObjectName>("water_fp") = "water_fp";
    uo_params.set<UserObjectName>("capillary_pressure") = "pc";
    _fe_problem->addUserObject("PorousFlowWaterVapor", "fp", uo_params);
    _fp = &_fe_problem->getUserObject<PorousFlowWaterVapor>("fp");
  }

  const PorousFlowCapillaryPressureVG * _pc;
  const PorousFlowWaterVapor * _fp;
  const Water97FluidProperties * _water_fp;
};
