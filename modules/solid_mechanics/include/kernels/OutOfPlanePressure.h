//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declarations
class Function;

/**
 * OutOfPlanePressure is a kernel used to apply pressure in the out-of-plane direction
 * in 2D plane stress or generalized plane strain models. Following the convention of
 * the standard Pressure boundary condition, positive pressures are applied inward into
 * the surface.
 */

class OutOfPlanePressure : public Kernel
{
public:
  static InputParameters validParams();

  OutOfPlanePressure(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

private:
  const PostprocessorValue * const _postprocessor;
  const Function & _function;
  const Real _factor;
};
