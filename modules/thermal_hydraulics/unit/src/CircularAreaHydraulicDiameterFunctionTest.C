//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CircularAreaHydraulicDiameterFunctionTest.h"
#include "FluidPropertiesTestUtils.h"

TEST_F(CircularAreaHydraulicDiameterFunctionTest, test)
{
  Function & fn = _fe_problem->getFunction(_Dh_name);

  const Point p(0.5, 0, 0);
  ABS_TEST(fn.value(0, p), 1.3819765978853, 1e-13);
}
