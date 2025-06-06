//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TestNumericalFlux1D.h"

class IdealGasMixtureFluidProperties;

/**
 * Base class for testing NumericalFluxGasMix objects
 */
class TestNumericalFluxGasMixBase : public TestNumericalFlux1D
{
public:
  TestNumericalFluxGasMixBase();

protected:
  virtual std::vector<ADReal> computeConservativeSolution(const std::vector<ADReal> & W,
                                                  const ADReal & A) const override;
  virtual std::vector<ADReal> computeFluxFromPrimitive(const std::vector<ADReal> & W,
                                               const ADReal & A) const override;

  /**
   * Adds fluid properties objects needed for testing
   */
  void addFluidProperties();

  /// Mixture fluid properties name
  const UserObjectName _fp_mix_name;

  /// Fluid properties user object
  const IdealGasMixtureFluidProperties * _fp_mix;
};
