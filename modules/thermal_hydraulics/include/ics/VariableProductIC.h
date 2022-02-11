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
