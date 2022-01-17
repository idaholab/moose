#pragma once

#include "THMControl.h"

/**
 * Control that multiplies old value by a scalar. Used for testing time dependent control values.
 */
class ScalingControl : public THMControl
{
public:
  ScalingControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Scaling factor
  const Real & _scale;
  /// Initial value
  const Real & _initial;
  /// Current value control
  Real & _value;
  /// Old value of the control
  const Real & _value_old;

public:
  static InputParameters validParams();
};
