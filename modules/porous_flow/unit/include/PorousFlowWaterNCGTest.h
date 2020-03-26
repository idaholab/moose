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

class PorousFlowWaterNCGTest : public MooseObjectUnitTest
{
public:
  PorousFlowWaterNCGTest() : MooseObjectUnitTest("PorousFlowApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters pc_params = _factory.getValidParams("PorousFlowCapillaryPressureVG");
    pc_params.set<Real>("m") = 0.5;
    pc_params.set<Real>("alpha") = 1.0e-4;
    _fe_problem->addUserObject("PorousFlowCapillaryPressureVG", "pc", pc_params);
    _pc = &_fe_problem->getUserObject<PorousFlowCapillaryPressureVG>("pc");

    InputParameters water_params = _factory.getValidParams("Water97FluidProperties");
    _fe_problem->addUserObject("Water97FluidProperties", "water_fp", water_params);
    _water_fp = &_fe_problem->getUserObject<SinglePhaseFluidProperties>("water_fp");

    InputParameters ncg_params = _factory.getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "ncg_fp", ncg_params);
    _ncg_fp = &_fe_problem->getUserObject<SinglePhaseFluidProperties>("ncg_fp");

    InputParameters uo_params = _factory.getValidParams("PorousFlowWaterNCG");
    uo_params.set<UserObjectName>("water_fp") = "water_fp";
    uo_params.set<UserObjectName>("gas_fp") = "ncg_fp";
    uo_params.set<UserObjectName>("capillary_pressure") = "pc";
    _fe_problem->addUserObject("PorousFlowWaterNCG", "fs", uo_params);
    _fs = &_fe_problem->getUserObject<PorousFlowWaterNCG>("fs");

    // Indices for derivatives
    _pidx = _fs->getPressureIndex();
    _Tidx = _fs->getTemperatureIndex();
    _Zidx = _fs->getZIndex();
  }

  const PorousFlowCapillaryPressureVG * _pc;
  const PorousFlowWaterNCG * _fs;
  const SinglePhaseFluidProperties * _water_fp;
  const SinglePhaseFluidProperties * _ncg_fp;
  unsigned int _pidx;
  unsigned int _Tidx;
  unsigned int _Zidx;
};
