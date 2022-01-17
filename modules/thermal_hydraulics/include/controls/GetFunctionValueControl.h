#pragma once

#include "THMControl.h"

class Function;

/**
 * This control takes a function and converts it into a control data
 */
class GetFunctionValueControl : public THMControl
{
public:
  GetFunctionValueControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// The stored function value
  Real & _value;
  /// Function that is sampled
  const Function & _function;

public:
  static InputParameters validParams();
};
