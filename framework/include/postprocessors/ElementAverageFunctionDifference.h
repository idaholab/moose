//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementAverageValue.h"

// Forward Declarations
class ElementAverageFunctionDifference;
class Function;

template <>
InputParameters validParams<ElementAverageFunctionDifference>();

/**
 * This postprocessor computes the difference between the elemental average value of a postprocessor
 * with a function
 */
class ElementAverageFunctionDifference : public ElementAverageValue
{
public:
  static InputParameters validParams();

  ElementAverageFunctionDifference(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// Function to be compared against
  const Function & _func;

  /// Flag to return the absolute value
  const bool _abs;
};
