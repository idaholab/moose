//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VolumePostprocessor.h"

/**
 * This postprocessor computes the contact area of a specified lower dimensional block.
 */
class MortarContactAreaPostprocessor : public VolumePostprocessor
{
public:
  static InputParameters validParams();

  MortarContactAreaPostprocessor(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Coupled Latrange Multiplier
  const ADVariableValue & _u;

  /// The tolerance used to decide whether the variable indicates contact
  const Real _tolerance;
};
