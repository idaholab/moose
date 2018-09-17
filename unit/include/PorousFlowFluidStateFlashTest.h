//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWFLUIDSTATEFLASHTEST_H
#define POROUSFLOWFLUIDSTATEFLASHTEST_H

#include "MooseObjectUnitTest.h"
#include "PorousFlowFluidStateFlash.h"

class PorousFlowFluidStateFlashTest : public MooseObjectUnitTest
{
public:
  PorousFlowFluidStateFlashTest() : MooseObjectUnitTest("MooseUnitApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_params = _factory.getValidParams("PorousFlowFluidStateFlash");
    _fe_problem->addUserObject("PorousFlowFluidStateFlash", "fp", uo_params);
    _fp = &_fe_problem->getUserObject<PorousFlowFluidStateFlash>("fp");
  }

  const PorousFlowFluidStateFlash * _fp;
};

#endif // POROUSFLOWFLUIDSTATEFLASHTEST_H
