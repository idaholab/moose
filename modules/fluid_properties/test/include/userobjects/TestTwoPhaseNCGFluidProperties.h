//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoPhaseNCGFluidProperties.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Test 2-phase NCG fluid properties
 *
 * This uses arbitrary functions for the two-phase interfaces.
 */
class TestTwoPhaseNCGFluidProperties : public TwoPhaseNCGFluidProperties
{
public:
  static InputParameters validParams();

  TestTwoPhaseNCGFluidProperties(const InputParameters & parameters);
};

#pragma GCC diagnostic pop
