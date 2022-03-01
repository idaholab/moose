//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "AuxKernel.h"

/**
 * Computes the min or max of element length.
 */
class ElementLengthAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ElementLengthAux(const InputParameters & parameters);

protected:
  /**
   * Returns the min/max of the current element.
   */
  virtual Real computeValue() override;

  /// The type of calculation to perform min or max
  const bool _use_min;
};
