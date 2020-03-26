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
#include "PorousFlowBrineCO2.h"
#include "BrineFluidProperties.h"
#include "Water97FluidProperties.h"
#include "NaClFluidProperties.h"
#include "CO2FluidProperties.h"

class PorousFlowBrineCO2Test : public MooseObjectUnitTest
{
public:
  PorousFlowBrineCO2Test() : MooseObjectUnitTest("PorousFlowApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters pc_params = _factory.getValidParams("PorousFlowCapillaryPressureVG");
    pc_params.set<Real>("m") = 0.5;
    pc_params.set<Real>("alpha") = 1.0e-4;
    _fe_problem->addUserObject("PorousFlowCapillaryPressureVG", "pc", pc_params);
    _pc = &_fe_problem->getUserObject<PorousFlowCapillaryPressureVG>("pc");

    InputParameters brine_params = _factory.getValidParams("BrineFluidProperties");
    _fe_problem->addUserObject("BrineFluidProperties", "brine_fp", brine_params);
    _brine_fp = &_fe_problem->getUserObject<BrineFluidProperties>("brine_fp");

    InputParameters water_params = _factory.getValidParams("Water97FluidProperties");
    _fe_problem->addUserObject("Water97FluidProperties", "water_fp", water_params);
    _water_fp = &_fe_problem->getUserObject<Water97FluidProperties>("water_fp");

    InputParameters co2_params = _factory.getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "co2_fp", co2_params);
    _co2_fp = &_fe_problem->getUserObject<SinglePhaseFluidProperties>("co2_fp");

    InputParameters uo_params = _factory.getValidParams("PorousFlowBrineCO2");
    uo_params.set<UserObjectName>("brine_fp") = "brine_fp";
    uo_params.set<UserObjectName>("co2_fp") = "co2_fp";
    uo_params.set<UserObjectName>("capillary_pressure") = "pc";
    _fe_problem->addUserObject("PorousFlowBrineCO2", "fs", uo_params);
    _fs = &_fe_problem->getUserObject<PorousFlowBrineCO2>("fs");

    // Indices for derivatives
    _pidx = _fs->getPressureIndex();
    _Tidx = _fs->getTemperatureIndex();
    _Zidx = _fs->getZIndex();
    _Xidx = _fs->getXIndex();
  }

  const PorousFlowCapillaryPressureVG * _pc;
  const PorousFlowBrineCO2 * _fs;
  const BrineFluidProperties * _brine_fp;
  const Water97FluidProperties * _water_fp;
  const SinglePhaseFluidProperties * _co2_fp;
  unsigned int _pidx;
  unsigned int _Tidx;
  unsigned int _Zidx;
  unsigned int _Xidx;
};
