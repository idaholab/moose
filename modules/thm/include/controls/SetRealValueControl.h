#pragma once

#include "THMControl.h"

/**
 * Control object that reads a Real value computed by the control logic system and sets it into a
 * specified MOOSE object parameter(s)
 */
class SetRealValueControl : public THMControl
{
public:
  SetRealValueControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// The value that is written into the MOOSE object's input parameter
  const Real & _value;

public:
  static InputParameters validParams();
};
