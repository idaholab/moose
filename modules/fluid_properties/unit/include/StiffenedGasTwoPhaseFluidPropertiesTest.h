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
#include "StiffenedGasTwoPhaseFluidProperties.h"
#include "StiffenedGasFluidProperties.h"

class StiffenedGasTwoPhaseFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  StiffenedGasTwoPhaseFluidPropertiesTest()
    : MooseObjectUnitTest("FluidPropertiesApp"), _T_triple(300.0), _L_fusion(0.5)
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("StiffenedGasTwoPhaseFluidProperties");
    uo_pars.set<Real>("T_triple") = _T_triple;
    uo_pars.set<Real>("L_fusion") = _L_fusion;
    _fe_problem->addUserObject("StiffenedGasTwoPhaseFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<StiffenedGasTwoPhaseFluidProperties>("fp");
    _fp_liquid = &_fe_problem->getUserObject<StiffenedGasFluidProperties>(_fp->getLiquidName());
    _fp_vapor = &_fe_problem->getUserObject<StiffenedGasFluidProperties>(_fp->getVaporName());
  }

  const StiffenedGasTwoPhaseFluidProperties * _fp;
  const StiffenedGasFluidProperties * _fp_liquid;
  const StiffenedGasFluidProperties * _fp_vapor;

  const Real _T_triple;
  const Real _L_fusion;
};
