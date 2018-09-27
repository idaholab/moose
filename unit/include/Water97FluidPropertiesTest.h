//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef WATER97FLUIDPROPERTIESTEST_H
#define WATER97FLUIDPROPERTIESTEST_H

#include "MooseObjectUnitTest.h"
#include "Water97FluidProperties.h"
#include "SinglePhaseFluidPropertiesPTTestUtils.h"

class Water97FluidPropertiesTest : public MooseObjectUnitTest
{
public:
  Water97FluidPropertiesTest() : MooseObjectUnitTest("MooseUnitApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("Water97FluidProperties");
    _fe_problem->addUserObject("Water97FluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<Water97FluidProperties>("fp");
  }

  const Water97FluidProperties * _fp;
};

#endif // WATER97FLUIDPROPERTIESTEST_H
