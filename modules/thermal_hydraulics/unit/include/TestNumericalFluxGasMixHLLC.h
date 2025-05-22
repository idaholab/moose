//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TestNumericalFluxGasMixBase.h"

/**
 * Tests NumericalFluxGasMixHLLC
 */
class TestNumericalFluxGasMixHLLC : public TestNumericalFluxGasMixBase
{
public:
  TestNumericalFluxGasMixHLLC();

protected:
  virtual const NumericalFlux1D & createFluxObject() override;

  virtual std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
  getPrimitiveSolutionsSymmetryTest() const override;

  virtual std::vector<std::vector<ADReal>> getPrimitiveSolutionsConsistencyTest() const override;
};
