//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "GeneralDamper.h"

/**
 * Simple constant damper.
 *
 * Modifies the non-linear step by applying a constant damping factor
 */
class ConstantDamper : public GeneralDamper
{
public:
  static InputParameters validParams();

  ConstantDamper(const InputParameters & parameters);

protected:
  /**
   * Return the constant damping value.
   */
  virtual Real computeDamping(const NumericVector<Number> & solution,
                              const NumericVector<Number> & update) override;

  /// The constant amount of the Newton update to take.
  Real _damping;
};
