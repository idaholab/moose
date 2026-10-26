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

class MortarContactPressureProvider;

/**
 * Integrates over a mortar interface the contact pressure that a MortarContactPressureProvider
 * supplies, interpolating its nodal pressures with its traction basis.
 */
class MortarContactPressureIntegral : public MortarUserObject
{
public:
  static InputParameters validParams();

  MortarContactPressureIntegral(const InputParameters & parameters);

  virtual void initialize() override { _integral = 0; }
  virtual void execute() override;
  virtual void finalize() override;
  virtual void reinit() override {}

  /// The integrated contact pressure
  Real integral() const { return _integral; }

protected:
  /// The object that provides the contact pressure
  const MortarContactPressureProvider & _provider;

  /// Member for handling change of coordinate systems (xyz, rz, spherical)
  const MooseArray<Real> & _coord;

  /// The integrated contact pressure
  Real _integral = 0;
};
