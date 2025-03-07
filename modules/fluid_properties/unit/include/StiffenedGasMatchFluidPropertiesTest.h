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

class SinglePhaseFluidProperties;

class StiffenedGasMatchFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  StiffenedGasMatchFluidPropertiesTest();

protected:
  void buildObjects();

  const Real _p;
  const Real _T;

  const SinglePhaseFluidProperties * _fp_ref;
  SinglePhaseFluidProperties * _fp_sgfit;
};
