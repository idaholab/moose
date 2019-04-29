#pragma once

#include "AuxKernel.h"

class SumAux;

template <>
InputParameters validParams<SumAux>();

/**
 * Sum of aux variables
 *
 * This computes a sum of aux variables:
 * \f[
 *   y_{total} = \sum\limits_i y_i .
 * \f]
 ]
 */
class SumAux : public AuxKernel
{
public:
  SumAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const unsigned int _n_values;
  std::vector<const VariableValue *> _values;
};
