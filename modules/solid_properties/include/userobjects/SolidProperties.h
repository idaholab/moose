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
// common in the SolidProperties module. This makes the computations much
// slower than necessary, so use a smaller definition in the SolidProperties
// module, FPDualReal, which is suitable for up to five variables.
// This is useful for the cases where we wish to use AD to compute the derivatives
// rather than hand-coding them in derived classes.
typedef DualNumber<Real, DNDerivativeSize<5>> FPDualReal;

class SolidProperties : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  SolidProperties(const InputParameters & parameters);

  virtual void execute() final {}
  virtual void initialize() final {}
  virtual void finalize() final {}

  virtual void threadJoin(const UserObject &) final {}
  virtual void subdomainSetup() final {}

protected:
  /// Flag to set unimplemented Jacobian entries to zero
  const bool _allow_imperfect_jacobians;
};
