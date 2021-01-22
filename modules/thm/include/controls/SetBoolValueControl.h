#pragma once

#include "THMControl.h"

/**
 * Control object that reads a boolean value computed by the control logic system and sets it into a
 * specified MOOSE object parameter(s)
 */
class SetBoolValueControl : public THMControl
{
public:
  SetBoolValueControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// The value that is written into the MOOSE object's input parameter
  const bool & _value;

public:
  static InputParameters validParams();
};
