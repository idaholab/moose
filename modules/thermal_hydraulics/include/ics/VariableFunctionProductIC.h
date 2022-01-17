#pragma once

#include "InitialCondition.h"

class Function;

/**
 * Computes product of a variable and a function
 */
class VariableFunctionProductIC : public InitialCondition
{
public:
  VariableFunctionProductIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Coupled variable
  const VariableValue & _var;
  /// Function
  const Function & _fn;

public:
  static InputParameters validParams();
};
