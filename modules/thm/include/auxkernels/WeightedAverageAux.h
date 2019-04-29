#pragma once

#include "AuxKernel.h"

class WeightedAverageAux;

template <>
InputParameters validParams<WeightedAverageAux>();

/**
 * Weighted average of an aux variable using another aux variable as the weights
 *
 * This computes a weighted average of aux variables:
 * \f[
 *   \bar{y} = \frac{\sum\limits_i w_i y_i}{\sum\limits_i w_i} .
 * \f]
 ]
 */
class WeightedAverageAux : public AuxKernel
{
public:
  WeightedAverageAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const unsigned int _n_values;
  std::vector<const VariableValue *> _values;
  std::vector<const VariableValue *> _weights;
};
