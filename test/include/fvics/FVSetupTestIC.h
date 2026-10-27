//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVInitialConditionTempl.h"
#include "FVSetupCounter.h"

/**
 * A constant finite volume initial condition that counts its initialSetup() calls.
 *
 * FVInitialConditionBase::initialSetup() is an empty virtual, so a missing dispatch to it is
 * silent; counting here is what makes it observable to a test through FVSetupCount.
 */
class FVSetupTestIC : public FVInitialCondition, public FVSetupCounter
{
public:
  static InputParameters validParams();

  FVSetupTestIC(const InputParameters & parameters);

  virtual void initialSetup() override { incrementSetupCount("INITIAL"); }
  virtual Real value(const Point & p) override;

protected:
  /// The value this initial condition imposes
  const Real _value;
};
