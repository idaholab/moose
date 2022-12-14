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
#include "Water97FluidProperties.h"
#include "SinglePhaseFluidProperties.h"

class Water97FluidPropertiesTest : public MooseObjectUnitTest
{
public:
  Water97FluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("Water97FluidProperties");
    // Give initial guesses that are slightly off
    uo_pars.set<Real>("T_initial_guess") = 298.15 * 1.01;
    uo_pars.set<Real>("p_initial_guess") = 1.01e5 * 1.01;
    _fe_problem->addUserObject("Water97FluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<Water97FluidProperties>("fp");

    InputParameters ad_uo_pars = _factory.getValidParams("Water97FluidProperties");
    _fe_problem->addUserObject("Water97FluidProperties", "ad_fp", ad_uo_pars);
    _ad_fp = &_fe_problem->getUserObject<SinglePhaseFluidProperties>("ad_fp");
  }

  const Water97FluidProperties * _fp;
  const SinglePhaseFluidProperties * _ad_fp;
};
