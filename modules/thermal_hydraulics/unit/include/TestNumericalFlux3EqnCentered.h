//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TestNumericalFlux3EqnBase.h"

/**
 * Tests NumericalFlux3EqnCentered.
 */
class TestNumericalFlux3EqnCentered : public TestNumericalFlux3EqnBase
{
protected:
  virtual const ADNumericalFlux3EqnBase * createFluxObject() override;

  virtual std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
  getPrimitiveSolutionsSymmetryTest() const override;

  virtual std::vector<std::vector<ADReal>> getPrimitiveSolutionsConsistencyTest() const override;
};
