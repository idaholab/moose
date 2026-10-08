//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TotalPowerBase.h"

/**
 * Prescribes total power via a user supplied value
 */
class TotalPower : public TotalPowerBase
{
public:
  TotalPower(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual Convergence * getNonlinearConvergence() const override { return nullptr; }

  /// The name of the function prescribing the power
  const FunctionName & _power_fn_name;

public:
  static InputParameters validParams();
};
