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
#include "ElementDamper.h"

/**
 * TODO
 */
class MaxIncrement : public ElementDamper
{
public:
  static InputParameters validParams();

  MaxIncrement(const InputParameters & parameters);

protected:
  virtual Real computeQpDamping() override;

  /// The maximum Newton increment for the variable.
  Real _max_increment;

  /* Enum for increment calculation type. Absolute compares the variable increment
   * to max_increment. Fractional compares the variable increment divided by the
   * variable value to to max_increment.
   */
  const enum class IncrementTypeEnum { absolute, fractional } _increment_type;
};
