//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometryUtils.h"
#include "MooseObjectUnitTest.h"

class GeometryUtilityTest : public MooseObjectUnitTest
{
public:
  GeometryUtilityTest() : MooseObjectUnitTest("MooseUnitApp") {}
};
