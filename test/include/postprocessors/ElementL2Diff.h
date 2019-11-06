//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

class ElementL2Diff : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  ElementL2Diff(const InputParameters & parameters);

protected:
  /**
   * Get the L2 Error.
   */
  virtual Real getValue();

  virtual Real computeQpIntegral();

  const VariableValue & _u_old;
};
