//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarUserObject.h"
#include "MortarContactPressureProvider.h"

/**
 * Provides a contact pressure stored in a nodal variable as a scaled value, the way a scaled
 * contact Lagrange multiplier stores it, and recovers the physical pressure by dividing by the
 * scaling factor.
 */
class TestMortarContactPressureProvider : public MortarUserObject,
                                          public MortarContactPressureProvider
{
public:
  static InputParameters validParams();

  TestMortarContactPressureProvider(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual void reinit() override {}

  virtual ADReal nodalContactPressure(const Node & node) const override;
  virtual const VariableTestValue & tractionBasis() const override;

protected:
  /// The variable that stores the scaled pressure
  const MooseVariable & _var;

  /// The factor by which the stored pressure exceeds the physical pressure
  const Real _scaling;
};
