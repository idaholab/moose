//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorInitialCondition.h"

// System includes
#include <string>

class InputParameters;

namespace libMesh
{
class Point;
}

/**
 * VectorConstantIC just returns a constant value.
 */
class VectorConstantIC : public VectorInitialCondition
{
public:
  static InputParameters validParams();

  VectorConstantIC(const InputParameters & parameters);

  virtual RealVectorValue value(const Point & p) override;

protected:
  const Real _x_value;
  const Real _y_value;
  const Real _z_value;
};
