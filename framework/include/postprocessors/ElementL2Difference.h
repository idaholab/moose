//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTL2DIFFERENCE_H
#define ELEMENTL2DIFFERENCE_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class ElementL2Difference;

template <>
InputParameters validParams<ElementL2Difference>();

/**
 * Computes the L2-Norm difference between two solution fields.
 */
class ElementL2Difference : public ElementIntegralVariablePostprocessor
{
public:
  ElementL2Difference(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;

  /// The variable to compare to
  const VariableValue & _other_var;
};

#endif // ELEMENTL2DIFFERENCE_H
