//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementExtremeFunctorValue.h"

/**
 * Computes a normalized variable step norm for various variables.
 */
class THMNormalizedVariableStep : public ElementExtremeFunctorValue
{
public:
  static InputParameters validParams();

  THMNormalizedVariableStep(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /// Normalization constant
  const Real _normalization;
};
