//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Abstract base class material providing the drag coefficients for linear and quadratic friction
 * models. The expression of the coefficients is highly dependent on the formulation of the kernel.
 * The reader should consult the PINSFVMomentumFrictionKernel documentation for details
 */
class FunctorDragCoefficients : public FunctorMaterial
{
public:
  FunctorDragCoefficients(const InputParameters & parameters);

  static InputParameters validParams();
};
