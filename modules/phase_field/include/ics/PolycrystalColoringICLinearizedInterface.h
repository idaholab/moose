//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolycrystalColoringIC.h"

// Forward Declarations
class PolycrystalColoringIC;

/**
 * PolycrystalColoringICLinearizedInterface creates a polycrystal initial condition from a user
 * object with linearized interface transformation
 */
class PolycrystalColoringICLinearizedInterface : public PolycrystalColoringIC
{
public:
  static InputParameters validParams();

  PolycrystalColoringICLinearizedInterface(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);
  const Real _bound;
};
