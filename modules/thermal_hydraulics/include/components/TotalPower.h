//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  /// The value of power
  const Real & _power;

public:
  static InputParameters validParams();
};
