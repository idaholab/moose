//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE Includes
#include "InitialCondition.h"

/**
 * ExampleIC just returns a constant value.
 */
class ExampleIC : public InitialCondition
{
public:
  /**
   * Constructor: Same as the rest of the MOOSE Objects
   */
  ExampleIC(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p) override;

private:
  const Real _coefficient;
};
