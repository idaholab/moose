//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

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

public:
  static InputParameters validParams();
};
