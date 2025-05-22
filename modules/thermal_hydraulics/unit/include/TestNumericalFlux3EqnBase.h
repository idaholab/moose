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

class SinglePhaseFluidProperties;

/**
 * Base class for testing numerical flux objects for the variable-area compressible Euler equations.
 */
class TestNumericalFlux3EqnBase : public TestNumericalFlux1D
{
public:
  TestNumericalFlux3EqnBase();

protected:
  virtual std::vector<ADReal> computeConservativeSolution(const std::vector<ADReal> & W,
                                                  const ADReal & A) const override;
  virtual std::vector<ADReal> computeFluxFromPrimitive(const std::vector<ADReal> & W,
                                               const ADReal & A) const override;

  /**
   * Builds and gets the fluid properties user object
   */
  const SinglePhaseFluidProperties & getFluidPropertiesObject();

  /// Fluid properties user object name
  const UserObjectName _fp_name;
  /// Fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};
