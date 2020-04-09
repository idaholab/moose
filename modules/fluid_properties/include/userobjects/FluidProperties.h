//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedGeneralUserObject.h"

// Forward Declarations

// The default DualReal size allows functions of many more variables than
// common in the FluidProperties module. This makes the computations much
// slower than necessary, so use a smaller definition in the FluidProperties
// module, FPDualReal, which is suitable for up to five variables.
// This is useful for the cases where we wish to use AD to compute the derivatives
// rather than hand-coding them in derived classes.
typedef DualNumber<Real, DNDerivativeSize<5>> FPDualReal;

class FluidProperties : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  FluidProperties(const InputParameters & parameters);
  virtual ~FluidProperties();

  virtual void execute() final {}
  virtual void initialize() final {}
  virtual void finalize() final {}

  virtual void threadJoin(const UserObject &) final {}
  virtual void subdomainSetup() final {}

  /// Universal gas constant (J/mol/K)
  static const Real _R;

protected:
  /// Conversion of temperature from Celsius to Kelvin
  const Real _T_c2k;
  /// Flag to set unimplemented Jacobian entries to zero
  const bool _allow_imperfect_jacobians;
};
