//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

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

public:
  static InputParameters validParams();
};
