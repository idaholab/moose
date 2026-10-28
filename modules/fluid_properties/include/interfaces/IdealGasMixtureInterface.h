//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

/**
 * Interface of the fluid properties of ideal gases that can be components of
 * IdealGasMixtureFluidProperties
 */
class IdealGasMixtureInterface
{
public:
  virtual ~IdealGasMixtureInterface() = default;

  /**
   * Whether the specific heats are constant, so that the specific internal energy is linear in
   * temperature
   */
  virtual bool hasConstantSpecificHeats() const = 0;

  /**
   * Temperature at which Newton's method starts to compute the temperature from the specific
   * internal energy [K]. A mixture starts at the lowest of these temperatures over its components
   * whose specific heats are not constant.
   */
  virtual Real newtonStartTemperature() const = 0;
};
