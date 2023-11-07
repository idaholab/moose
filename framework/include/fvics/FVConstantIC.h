//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVInitialConditionTempl.h"

class InputParameters;

namespace libMesh
{
class Point;
}

/**
 * FVConstantIC just returns a constant value for a finite volume variable.
 */
class FVConstantIC : public FVInitialCondition
{
public:
  static InputParameters validParams();

  FVConstantIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  const Real _value;
};
