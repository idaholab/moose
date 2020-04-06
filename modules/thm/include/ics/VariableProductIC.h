#pragma once

#include "InitialCondition.h"

/**
 * Computes the product of coupled variables
 */
class VariableProductIC : public InitialCondition
{
public:
  VariableProductIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  /// The number of coupled variables
  unsigned int _n;
  /// The values being multipled
  std::vector<const VariableValue *> _values;

public:
  static InputParameters validParams();
};
