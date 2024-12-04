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

class IdealGasMixtureFluidProperties;

/**
 * Tests FlowModelGasMixUtils
 */
class FlowModelGasMixUtilsTest : public MooseObjectUnitTest
{
public:
  FlowModelGasMixUtilsTest();

protected:
  /**
   * Adds fluid properties objects needed for testing
   */
  void addFluidProperties();

  /// Fluid properties user object
  const IdealGasMixtureFluidProperties * _fp_mix;
};
