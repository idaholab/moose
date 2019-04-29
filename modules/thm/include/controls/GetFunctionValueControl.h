#pragma once

#include "THMControl.h"

class GetFunctionValueControl;
class Function;

template <>
InputParameters validParams<GetFunctionValueControl>();

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
  Function & _function;
};
