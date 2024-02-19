//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class ADShaftConnectedCompressor1PhaseUserObject;

/**
 * Gets various quantities for a ShaftConnectedCompressor1Phase
 */
class ShaftConnectedCompressor1PhasePostprocessor : public GeneralPostprocessor
{
public:
  ShaftConnectedCompressor1PhasePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() const override;

protected:
  /// Quantity type
  enum class Quantity
  {
    PRESSURE_RATIO,
    EFFICIENCY,
    REL_CORRECTED_FLOW,
    REL_CORRECTED_SPEED
  };
  /// Quantity to get
  const Quantity _quantity;

  /// 1-phase shaft-connected compressor user object
  const ADShaftConnectedCompressor1PhaseUserObject & _compressor_uo;

public:
  static InputParameters validParams();
};
