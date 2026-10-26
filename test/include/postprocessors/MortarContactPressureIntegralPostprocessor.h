//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class MortarContactPressureIntegral;

/**
 * Reports the integral computed by a MortarContactPressureIntegral. Mortar user objects do not
 * store postprocessor values, so a general postprocessor exposes the result for output.
 */
class MortarContactPressureIntegralPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  MortarContactPressureIntegralPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

protected:
  /// The user object that computes the integral
  const MortarContactPressureIntegral & _integral;
};
