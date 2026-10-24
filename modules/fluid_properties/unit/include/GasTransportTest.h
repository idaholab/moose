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
#include "Water97FluidProperties.h"

class GasTransportTest : public MooseObjectUnitTest
{
public:
  GasTransportTest();

protected:
  /// Water and steam properties, the reference for the low-density properties of steam
  const Water97FluidProperties * _water;
};

inline GasTransportTest::GasTransportTest() : MooseObjectUnitTest("FluidPropertiesApp")
{
  InputParameters params = _factory.getValidParams("Water97FluidProperties");
  _fe_problem->addUserObject("Water97FluidProperties", "water", params);
  _water = &_fe_problem->getUserObject<Water97FluidProperties>("water");
}
