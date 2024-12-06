//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainControl.h"

/**
 * Trips a boolean value if an input boolean value is a certain value.
 */
class UnitTripChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  UnitTripChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Whether to trip on true or false value
  const bool _trip_on_true;
  /// Value to check for trip
  const bool & _input;
  /// Tripped status
  bool & _tripped;
};
