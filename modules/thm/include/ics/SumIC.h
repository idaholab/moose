#pragma once

#include "InitialCondition.h"

class SumIC;

template <>
InputParameters validParams<SumIC>();

/**
 * Sum of aux variables
 *
 * This computes a sum of aux variables:
 * \f[
 *   y_{total} = \sum\limits_i y_i .
 * \f]
 ]
 */
class SumIC : public InitialCondition
{
public:
  SumIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p) override;

  /// number of values to sum
  const unsigned int _n_values;
  /// vector of pointers to values to sum
  std::vector<const VariableValue *> _values;
};
